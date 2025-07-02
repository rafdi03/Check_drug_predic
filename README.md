# Check Drug Predict

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Python](https://img.shields.io/badge/Python-3.7%2B-blue.svg)](https://www.python.org/)
[![Stars](https://img.shields.io/github/stars/rafdi03/Check_drug_predic?style=social)](https://github.com/rafdi03/Check_drug_predic/stargazers)

---

## ✨ Description

**Check Drug Predict** is an advanced open-source system that predicts the most suitable drug recommendation based on patient data or specific medical features. By leveraging modern machine learning and deep learning (VGG16) models, this application empowers healthcare professionals, researchers, and enthusiasts to rapidly and accurately identify optimal medication recommendations for individuals.

It includes a real-time AI-powered vision system, supporting live camera feed and object recognition, and features a visually appealing, modern web interface.

---

## 🚀 Key Features

- **Automatic Drug Prediction:**  
  Instantly predicts the best drug based on patient input data (age, sex, blood pressure, cholesterol, etc).
- **AI Vision System:**  
  Real-time camera-based object recognition using VGG16 deep learning model.
- **Data Analysis:**  
  Provides visual and statistical insights from patient data and prediction results.
- **User-Friendly Interface:**  
  Modern, aesthetic, and responsive web UI with live updates and prediction history.
- **Integrated Machine Learning Models:**  
  Utilizes up-to-date ML and DL models for high-accuracy prediction.
- **Easily Extendable:**  
  Modular Python codebase and clean project structure for future development.
- **Hardware Integration:**  
  Includes ESP32 microcontroller support for live image capture and communication.
- **Open Source:**  
  Community-driven and MIT licensed.

---

## 🏗️ Project Structure

```
Check_drug_predic/
│
├── data/               # Datasets and supporting files
├── models/             # Pre-trained machine learning models
├── src/                # Main source code (prediction logic, utils, etc)
│   ├── utils.py
│   ├── predictor.py
│   └── ...
├── static/             # CSS, JS, and static assets for the web interface
├── templates/          # HTML templates (Flask-based web UI)
├── ESP32/              # ESP32 microcontroller firmware (Arduino)
├── requirements.txt    # Python dependencies
├── app.py              # Main Flask app (backend)
├── README.md           # Project documentation
└── ...
```

---

## ⚙️ Installation

1. **Clone this repository**
    ```bash
    git clone https://github.com/rafdi03/Check_drug_predic.git
    cd Check_drug_predic
    ```

2. **(Optional but recommended) Create a virtual environment**
    ```bash
    python -m venv venv
    source venv/bin/activate  # Linux/macOS
    venv\Scripts\activate     # Windows
    ```

3. **Install dependencies**
    ```bash
    pip install -r requirements.txt
    ```

4. **Run the application**
    ```bash
    python src/main.py 
    # or
    flask run
    # or
    python app.py
    ```

---

## 🖥️ Web Interface

The web UI features:

- **Live Camera Feed** (real-time via ESP32 or webcam)
- **AI Prediction**: VGG16 model for image-based drug/object classification.
- **Prediction History**: Interactive, scrollable view of past predictions with images and confidence scores.
- **Modern, Aesthetic Design**: Responsive layout, smooth transitions, dark mode, and custom icons.

Try it by accessing [http://localhost:5000](http://localhost:5000) after running the application!

---

## 🤖 Hardware Integration

This project supports ESP32 microcontroller for live image capture:

- Firmware is located in `/ESP32/Final_Obat.ino`.
- Sends captured images to the Flask backend for AI-based prediction.
- Receives prediction results back to trigger hardware actions (e.g., LED, audio feedback).

---

## 📊 Dataset

Uses medical datasets (e.g.: [Drug Classification Dataset](https://www.kaggle.com/datasets/gauravduttakiit/drug-classification)) with features such as age, sex, blood pressure, cholesterol, etc.  
**Note:** Adjust your dataset format as needed for your use case.

---

## 💡 Contribution

Contributions are very welcome!  
Feel free to fork, create a new branch, and submit a pull request for new features or improvements.

Please read [CONTRIBUTING.md](CONTRIBUTING.md) before starting to contribute.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

---

## 📬 Contact & Support

- **Author**: [rafdi03](https://github.com/rafdi03)
- **Email**: rafdi03@gmail.com
- **Discussion & Issues**: [GitHub Issues](https://github.com/rafdi03/Check_drug_predic/issues)

---

> _“With technology, health prediction becomes easier and faster.”_

---

### 🌈 Screenshots

![Main UI](static/images/demo_main_ui.png)
![Prediction History](static/images/demo_history.png)
![ESP32 Hardware](static/images/demo_esp32.jpg)

---

### 🎨 Aesthetic & UX Highlights

- **Frosted glass** backgrounds, gradients, and glowing accents.
- **Live updating** camera and prediction panels with animated loading.
- **Intuitive** navigation and layout.
- **Dark mode** by default for eye comfort.

---
