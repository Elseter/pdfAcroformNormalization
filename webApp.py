import os
import requests
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/')
def index():
    return 'Hello, World!'

@app.route('/download_pdf', methods=['POST'])
def download_pdf():
    data = request.get_json()

    # Check to see if URL is in the POST request
    if 'url' not in data:
        return jsonify({'error': 'Missing URL'}), 400

    url = data['url']
    pdf_dir = 'build/'
    os.makedirs(pdf_dir, exist_ok=True)
    pdf_path = os.path.join(pdf_dir, 'downloaded.pdf')

    try:
        response = requests.get(url)
        response.raise_for_status()

        with open(pdf_path, 'wb') as f:
            f.write(response.content)

    except requests.exceptions.RequestException as e:
        return jsonify({'error': str(e)}), 500

    # run the normCPP script
    os.system('/app/build/normCPP downloaded.pdf normed.pdf')
    return jsonify({'message': 'PDF downloaded and processed successfully'}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)