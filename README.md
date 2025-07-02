# Check Drug Predict

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Python](https://img.shields.io/badge/Python-3.7+-blue.svg)](https://www.python.org/)
[![Stars](https://img.shields.io/github/stars/rafdi03/Check_drug_predic?style=social)](https://github.com/rafdi03/Check_drug_predic/stargazers)

---

## ✨ Deskripsi

**Check Drug Predict** adalah sebuah proyek open-source yang bertujuan untuk melakukan prediksi penggunaan obat berdasarkan data pasien atau fitur medis tertentu. Dengan menggabungkan teknologi machine learning, aplikasi ini memudahkan tenaga medis atau peneliti untuk mengidentifikasi jenis obat yang sesuai untuk pasien secara cepat dan akurat.

---

## 🚀 Fitur Utama

- **Prediksi Otomatis Obat**  
  Memprediksi jenis obat berdasarkan data input pasien.
- **Antarmuka Sederhana & User-Friendly**  
  Mudah digunakan oleh pengguna baru maupun berpengalaman.
- **Analisis Data**  
  Menyediakan insight dari data pasien dan hasil prediksi.
- **Model Machine Learning Terintegrasi**  
  Menggunakan model ML terkini untuk meningkatkan akurasi prediksi.
- **Ekstensi Mudah**  
  Struktur kode modular memudahkan pengembangan lebih lanjut.

---

## 🏗️ Struktur Proyek

```
Check_drug_predic/
│
├── data/               # Dataset dan file pendukung
├── models/             # Model machine learning terlatih
├── src/                # Kode sumber utama aplikasi
│   ├── utils.py
│   ├── predictor.py
│   └── ...
├── requirements.txt    # Daftar dependensi Python
├── README.md           # Dokumentasi proyek
└── ...
```

---

## ⚙️ Cara Instalasi

1. **Clone repo ini**
    ```bash
    git clone https://github.com/rafdi03/Check_drug_predic.git
    cd Check_drug_predic
    ```

2. **Aktifkan virtual environment (opsional tapi disarankan)**
    ```bash
    python -m venv venv
    source venv/bin/activate  # Linux/macOS
    venv\Scripts\activate     # Windows
    ```

3. **Install dependencies**
    ```bash
    pip install -r requirements.txt
    ```

4. **Jalankan aplikasi**
    ```bash
    python src/main.py
    ```

---

## 🧑‍💻 Contoh Penggunaan

```python
from src.predictor import predict_drug

# Contoh data pasien
patient_data = {
    'age': 45,
    'sex': 'F',
    'blood_pressure': 'HIGH',
    'cholesterol': 'NORMAL',
    # ... fitur lain
}

result = predict_drug(patient_data)
print(f"Prediksi obat yang direkomendasikan: {result}")
```

---

## 📊 Dataset

Proyek ini menggunakan dataset medis (misal: [Drug Classification Dataset](https://www.kaggle.com/datasets/gauravduttakiit/drug-classification)) yang berisi fitur-fitur seperti usia, jenis kelamin, tekanan darah, kolesterol, dan lain-lain. Pastikan untuk menyesuaikan format dataset sesuai kebutuhan aplikasi.

---

## 💡 Kontribusi

Kontribusi sangat terbuka!  
Silakan lakukan fork, buat branch baru, dan ajukan pull request jika ingin menambahkan fitur atau perbaikan.  
Pastikan untuk membaca [CONTRIBUTING.md](CONTRIBUTING.md) sebelum mulai berkontribusi.

---

## 📄 Lisensi

Proyek ini dilisensikan di bawah [MIT License](LICENSE).

---

## 📬 Kontak & Dukungan

- **Author**: [rafdi03](https://github.com/rafdi03)
- **Email**: rafdi03@gmail.com
- **Diskusi**: Gunakan [Issues](https://github.com/rafdi03/Check_drug_predic/issues) untuk bertanya dan berdiskusi.

---
