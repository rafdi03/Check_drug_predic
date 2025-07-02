# Check Drug Predict

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Python](https://img.shields.io/badge/Python-3.7+-blue.svg)](https://www.python.org/)
[![Stars](https://img.shields.io/github/stars/rafdi03/Check_drug_predic?style=social)](https://github.com/rafdi03/Check_drug_predic/stargazers)

---

## ✨ Description

**Check Drug Predict** is an open-source project designed to predict drug recommendations based on patient data or specific medical features. By leveraging machine learning technology, this application enables healthcare professionals or researchers to rapidly and accurately identify the most suitable medication for a patient.

---

## 🚀 Key Features

- **Automatic Drug Prediction**  
  Predicts the most suitable drug based on patient input data.
- **Simple & User-Friendly Interface**  
  Easy to use for both beginners and experienced users.
- **Data Analysis**  
  Provides insights from patient data and prediction results.
- **Integrated Machine Learning Models**  
  Uses up-to-date ML models to improve prediction accuracy.
- **Easily Extendable**  
  Modular code structure for easy further development.

---

## 🏗️ Project Structure

```
Check_drug_predic/
│
├── data/               # Datasets and supporting files
├── models/             # Pre-trained machine learning models
├── src/                # Main source code
│   ├── utils.py
│   ├── predictor.py
│   └── ...
├── requirements.txt    # Python dependencies
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
    python flask run # u also can using this if get error 
    python app.py    # or this 
    ```

---

## 🧑‍💻 Usage Example

```python
from src.predictor import predict_drug

# Example patient data
patient_data = {
    'age': 45,
    'sex': 'F',
    'blood_pressure': 'HIGH',
    'cholesterol': 'NORMAL',
    # ... other features
}

result = predict_drug(patient_data)
print(f"Recommended drug prediction: {result}")
```

---

## 📊 Dataset

This project uses a medical dataset (e.g.: [Drug Classification Dataset](https://www.kaggle.com/datasets/gauravduttakiit/drug-classification)) containing features such as age, sex, blood pressure, cholesterol, and others. Make sure to adjust your dataset format as needed.

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
- **Discussion**: Use [Issues](https://github.com/rafdi03/Check_drug_predic/issues) for questions and discussions.

---

> _“With technology, health prediction becomes easier and faster.”_
