from flask import Blueprint, jsonify
from log import Log
from config import Configuration
import pickle

api = Blueprint('api', __name__, template_folder = 'templates')

def unpickle_db():
    db_file = open('config.out', 'rb')
    return pickle.load(db_file)

def unpickle_log():
    log_file = open('log.out', 'rb')
    return pickle.load(log_file)


#Modify/fetch setting data
@api.route('/set-sensitivity', methods=['POST'])
def set_sensitivity():
    new_sensitivity = request.form.get('data')
    settings = unpickle_db()
    settings.sensitivity = new_sensitivity
    pickle.dump(settings, open('config.out', 'wb'))
    return ('ok', 200)

@api.route('/set-angle-limit', methods=['POST'])
def set_angle_limit():
    new_limit = request.form.get('data')
    settings = unpickle_db()
    settings.angle_limit = new_limit
    pickle.dump(settings, open('config.out', 'wb'))
    return ('ok', 200)

@api.route('/set-shutoff-duration', methods=['POST'])
def set_shutoff_duration():
    new_limit = request.form.get('data')
    settings = unpickle_db()
    settings.shutoff_duration = new_duration
    pickle.dump(settings, open('config.out', 'wb'))
    return ('ok', 200)


@api.route('/get-settings', methods=['GET'])
def get_settings():
    settings = unpickle_db()
    res = jsonify ({
        'gesture-sensitivity' : settings.gesture_rate,
        'angle-limit' : settings.angle_limit,
        'shutoff-duration' : settings.shutoff_duration
    })
    return res


#Send logging data
@api.route('/log', methods=['POST'])
def log():
    time = request.form.get('time')
    is_open = request.form.get('is-open')
    open_angle = request.form.get('open-angle')
    
    put = Log(time, is_open, open_angle)

    curr_logs = unpickle_log()
    curr_logs.append(put)

    pickle.dumps(curr_logs, open('log.out', 'wb'))
