from flask import Flask, render_template, request, send_from_directory, jsonify
import os
import time
import tensorflow as tf
import numpy as np
from PIL import Image
import io
import json
import shutil
import traceback

UPLOAD_FOLDER = 'static/images'
HISTORY_FILE = 'prediction_history.txt'
MODEL_WEIGHTS_PATH = r'C:\Users\aryag\Documents\GitHub\Check_drug_predic\models\best_vgg16_model (8).h5'
CLASS_MAPPING_FILE = r'C:\Users\aryag\Documents\GitHub\Check_drug_predic\Class\class_indices_mapping.json'
MODEL_INPUT_SIZE = (128, 128)

DENSE_UNITS_1 = 128
DROPOUT_RATE_1 = 0.4
DENSE_UNITS_2 = 64
DROPOUT_RATE_2 = 0.3
regularizer_l2 = None
USE_BATCH_NORM = False

MAX_HISTORY_ITEMS = 10

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

if os.path.exists(UPLOAD_FOLDER):
    pass
else:
    os.makedirs(UPLOAD_FOLDER)
    print(f"Direktori upload dibuat: {UPLOAD_FOLDER}")

latest_prediction_html = "Inisialisasi model ML..."
last_update_time = 0
last_image_filename = None
best_prediction_index_for_arduino = -1
best_prediction_score_for_esp32 = 0.0

prediction_history = []

image_counter = 0
if os.path.exists(UPLOAD_FOLDER):
    existing_images = [f for f in os.listdir(UPLOAD_FOLDER) if f.startswith('image_') and f.lower().endswith(('.png', '.jpg', '.jpeg'))]
    if existing_images:
        max_num = -1
        for img_name in existing_images:
            try:
                parts = img_name.replace('.', '_').split('_')
                if len(parts) > 1 and parts[1].isdigit():
                    max_num = max(max_num, int(parts[1]))
            except Exception:
                continue
        image_counter = max_num
        if max_num >= 0:
            latest_existing_file = f'image_{max_num:04d}.jpg'
            if os.path.exists(os.path.join(UPLOAD_FOLDER, latest_existing_file)):
                last_image_filename = latest_existing_file
                try:
                    last_update_time = os.path.getmtime(os.path.join(UPLOAD_FOLDER, last_image_filename))
                    print(f"Terakhir upload terdeteksi: {last_image_filename} ({time.ctime(last_update_time)})")
                except Exception:
                    pass
            print(f"Melanjutkan penamaan gambar dari nomor urut: {image_counter + 1}")
    else:
        print("Tidak ada gambar lama ditemukan di folder upload.")

model = None
idx_to_class = None
num_classes = 0

try:
    if os.path.exists(CLASS_MAPPING_FILE):
        print(f"[INFO] Memuat mapping kelas dari: {CLASS_MAPPING_FILE}")
        with open(CLASS_MAPPING_FILE, 'r') as f:
            idx_to_class_str_keys = json.load(f)
            idx_to_class = {int(k): v for k, v in idx_to_class_str_keys.items()}
        num_classes = len(idx_to_class)
        sorted_class_names = [idx_to_class[i] for i in sorted(idx_to_class.keys())]
        print(f"[INFO] Mapping kelas berhasil dimuat. Jumlah kelas: {num_classes}")
        print(f"[INFO] Urutan Kelas (berdasarkan indeks): {sorted_class_names}")

        print("\n[INFO] Membangun ulang arsitektur model VGG16...")
        from tensorflow.keras.applications import VGG16
        from tensorflow.keras.models import Model
        from tensorflow.keras.layers import GlobalAveragePooling2D, Dense, Dropout, Flatten
        if USE_BATCH_NORM:
            from tensorflow.keras.layers import BatchNormalization
        if regularizer_l2 is not None:
            from tensorflow.keras.regularizers import l2

        base_model_vgg16 = VGG16(
            weights='imagenet',
            include_top=False,
            input_shape=(MODEL_INPUT_SIZE[0], MODEL_INPUT_SIZE[1], 3)
        )
        base_model_vgg16.trainable = False

        x = base_model_vgg16.output
        x = GlobalAveragePooling2D()(x)

        dense_layer_1 = Dense(units=DENSE_UNITS_1, activation='relu',
                              kernel_regularizer=regularizer_l2
                             )
        x = dense_layer_1(x)
        if USE_BATCH_NORM:
            x = BatchNormalization()(x)
        x = Dropout(DROPOUT_RATE_1)(x)

        dense_layer_2 = Dense(units=DENSE_UNITS_2, activation='relu',
                              kernel_regularizer=regularizer_l2
                             )
        x = dense_layer_2(x)
        if USE_BATCH_NORM:
            x = BatchNormalization()(x)
        x = Dropout(DROPOUT_RATE_2)(x)

        predictions = Dense(units=num_classes, activation='softmax')(x)
        model = Model(inputs=base_model_vgg16.input, outputs=predictions)
        print("[INFO] Arsitektur model VGG16 berhasil dibangun kembali.")

        if os.path.exists(MODEL_WEIGHTS_PATH):
            print(f"[INFO] Memuat bobot model dari: {MODEL_WEIGHTS_PATH}")
            model.load_weights(MODEL_WEIGHTS_PATH)
            print("[✅] Bobot model berhasil dimuat.")
            latest_prediction_html = "Model siap. Menunggu gambar untuk diprediksi..."
        else:
            print(f"[❌] ERROR: File model tidak ditemukan di {MODEL_WEIGHTS_PATH}")
            latest_prediction_html = f"[❌] ERROR: File model tidak ditemukan ({MODEL_WEIGHTS_PATH})."
            model = None
    else:
        print(f"[❌] ERROR: File mapping kelas tidak ditemukan di {CLASS_MAPPING_FILE}")
        latest_prediction_html = f"[❌] ERROR: File mapping kelas tidak ditemukan ({CLASS_MAPPING_FILE})."
        model = None
except Exception as e:
    print(f"[❌] ERROR saat memuat arsitektur atau bobot model: {e}")
    traceback.print_exc()
    latest_prediction_html = f"[❌] ERROR saat memuat model: {e}"
    model = None

def predict_image(image_path):
    global model, idx_to_class, num_classes
    if model is None or idx_to_class is None or num_classes == 0:
        return "Model ML tidak tersedia atau mapping kelas error.", None, -1, 0.0

    try:
        img = Image.open(image_path).convert('RGB')
        img = img.resize(MODEL_INPUT_SIZE)
        img_array = np.array(img)
        img_array = np.expand_dims(img_array, axis=0)
        processed_img = img_array / 255.0
        predictions = model.predict(processed_img)
        top_indices = np.argsort(predictions[0])[-3:][::-1]
        top_scores = predictions[0][top_indices]
        result_text = "Prediksi:"
        decoded_predictions_for_log = []

        best_index = -1
        best_score = 0.0
        if len(top_indices) > 0:
            best_index = int(top_indices[0])
            best_score = float(top_scores[0])

        for i, idx in enumerate(top_indices):
            label = idx_to_class.get(idx, f"Unknown Index {idx}")
            score = top_scores[i]
            result_text += f"<br>{i+1}. {label} ({score:.4f})"
            decoded_predictions_for_log.append((int(idx), label, float(score)))

        return result_text, decoded_predictions_for_log, best_index, best_score
    except FileNotFoundError:
        return "ERROR: File gambar tidak ditemukan untuk prediksi.", None, -1, 0.0
    except Exception as e:
        print(f"[❌] ERROR saat melakukan prediksi: {e}")
        traceback.print_exc()
        return f"[❌] ERROR saat prediksi: {e}", None, -1, 0.0

def log_prediction_history(filename, prediction_data, best_index=None, best_score=None):
    try:
        timestamp_str = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
        log_entry = f"[{timestamp_str}] File: {filename}\n"
        if best_index is not None and best_index != -1:
            log_entry += f"    Best Prediction Index (for Arduino): {best_index}\n"
        if best_score is not None:
            log_entry += f"    Best Prediction Score (for ESP32): {best_score:.4f}\n"

        if prediction_data is None:
            log_entry += "    Prediksi: Gagal melakukan prediksi atau model tidak tersedia.\n"
        elif isinstance(prediction_data, list) and all(isinstance(item, tuple) for item in prediction_data):
            log_entry += "    Prediksi:\n"
            for i, (pred_id, label, score) in enumerate(prediction_data):
                log_entry += f"      - Top {i+1} (Index {pred_id}): {label} (Skor: {score:.4f})\n"
        else:
            log_entry += f"    Detail/Error: {prediction_data}\n"
        log_entry += "-"*30 + "\n"
        with open(HISTORY_FILE, 'a', encoding='utf-8') as f:
            f.write(log_entry)
    except Exception as e:
        print(f"[❌] ERROR saat mencatat history: {e}")
        traceback.print_exc()

@app.route('/upload', methods=['POST'])
def upload_image():
    global image_counter, latest_prediction_html, last_update_time, last_image_filename, \
            best_prediction_index_for_arduino, best_prediction_score_for_esp32, prediction_history

    if model is None or idx_to_class is None:
        print("[WARNING] Upload ditolak: Model belum siap atau gagal dimuat.")
        return jsonify({
            "status": "error",
            "message": "Model not ready for prediction.",
            "index": -1,
            "score": 0.0
        }), 503

    if request.headers.get('Content-Type') == 'image/jpeg':
        image_data = request.data

        if not image_data:
            print("[WARNING] Upload diterima tapi data kosong.")
            return jsonify({
                "status": "error",
                "message": "No image data received.",
                "index": -1,
                "score": 0.0
            }), 400

        try:
            image_counter += 1
            filename = f'image_{image_counter:04d}.jpg'
            image_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)

            with open(image_path, 'wb') as f:
                f.write(image_data)
            print(f"[📸] Gambar baru diterima dan disimpan sebagai: {filename}")
            
            html_pred, raw_pred_log, best_idx, best_scr = predict_image(image_path)
            
            current_time = time.time()
            latest_prediction_html = html_pred
            last_update_time = current_time
            last_image_filename = filename
            best_prediction_index_for_arduino = best_idx
            best_prediction_score_for_esp32 = best_scr

            log_prediction_history(filename, raw_pred_log, best_idx, best_scr)

            history_item = {
                'filename': filename,
                'timestamp': current_time,
                'prediction_html': html_pred,
                'prediction_raw': raw_pred_log
            }
            prediction_history.insert(0, history_item)
            if len(prediction_history) > MAX_HISTORY_ITEMS:
                prediction_history.pop()

            return jsonify({
                "status": "success",
                "message": "Image received and processed.",
                "index": best_idx,
                "score": f"{best_scr:.4f}"
            }), 200

        except Exception as e:
            print(f"[❌] ERROR saat menyimpan atau memproses gambar (setelah menerima data): {e}")
            traceback.print_exc()
            log_prediction_history("unknown_filename_error", f"Processing ERROR (after receiving data): {e}")
            return jsonify({
                "status": "error",
                "message": f"Image received, but processing failed: {e}",
                "index": -1,
                "score": 0.0
            }), 500

    else:
        print(f"[WARNING] Upload diterima dengan Content-Type salah: {request.headers.get('Content-Type')}")
        return jsonify({
            "status": "error",
            "message": "Invalid Content-Type. Expected image/jpeg.",
            "index": -1,
            "score": 0.0
        }), 415

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/latest_data')
def get_latest_data():
    global latest_prediction_html, last_update_time, last_image_filename
    image_url = ''
    if last_image_filename:
        image_url = f'/static/images/{last_image_filename}?t={int(last_update_time)}'
    return jsonify({
        'image_url': image_url,
        'prediction': latest_prediction_html,
        'timestamp': last_update_time,
        'timestamp_formatted': time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(last_update_time)) if last_update_time > 0 else "N/A",
        'filename': last_image_filename
    })

@app.route('/history_data')
def get_history_data():
    global prediction_history
    formatted_history = []
    for item in prediction_history:
        structured_predictions = []
        if item['prediction_raw']:
            for idx, label, score in item['prediction_raw']:
                structured_predictions.append({
                    'index': idx,
                    'label': label,
                    'score': score
                })
        formatted_history.append({
            'image_url': f'/static/images/{item["filename"]}?t={int(item["timestamp"])}',
            'timestamp_formatted': time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(item["timestamp"])),
            'predictions': structured_predictions
        })
    return jsonify(formatted_history)

@app.route('/arduino_best_prediction')
def get_arduino_best_prediction():
    global best_prediction_index_for_arduino, last_image_filename, last_update_time
    if last_image_filename is None or best_prediction_index_for_arduino == -1 or model is None:
        return "-1", 200
    return str(best_prediction_index_for_arduino), 200

@app.route('/esp32_best_score')
def get_esp32_best_score():
    global best_prediction_score_for_esp32, last_image_filename, model

    if last_image_filename is None or model is None:
        return "0.0000", 200

    return f"{best_prediction_score_for_esp32:.4f}", 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)