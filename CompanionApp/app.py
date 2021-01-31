from flask import Flask, render_template
from api import api
from config import Configuration
import pickle

app = Flask(__name__, static_folder='static')

app.register_blueprint(api, url_prefix='/api')

def initConfig():
    config = Configuration(200, 90, 10000)
    db_file = open('config.out', 'wb')
    pickle.dump(config, db_file)
    db_file.close()

def initLog():
    logs = []
    log_file = open('log.out', 'wb')
    pickle.dump(logs, log_file)
    log_file.close()

@app.route('/', methods=['GET'])
def home():
    return render_template('index.html')

@app.route('/settings', methods=['GET'])
def settings():
    return render_template('settings.html')


if __name__ == '__main__':
    initConfig()
    initLog()
    app.run('0.0.0.0', 5000)