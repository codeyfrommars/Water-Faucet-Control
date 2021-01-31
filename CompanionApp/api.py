from flask import Blueprint, jsonify, request
from log import Log
from config import Configuration
import pickle

api = Blueprint('api', __name__, template_folder = 'templates')

def unpickle_db():
    db_file = open('config.out', 'rb')
    res = pickle.load(db_file)
    db_file.close()
    return res

def unpickle_log():
    log_file = open('log.out', 'rb')
    res = pickle.load(log_file)
    log_file.close()
    return res


#Modify/fetch setting data
@api.route('/set-sensitivity', methods=['POST'])
def set_sensitivity():
    data_req = request.get_json()
    new_sensitivity = data_req['data']
    settings = unpickle_db()
    settings.sensitivity = new_sensitivity
    pickle.dump(settings, open('config.out', 'wb'))
    return ('ok', 200)

@api.route('/set-angle-limit', methods=['POST'])
def set_angle_limit():
    data_req = request.get_json()
    new_limit = data_req['data']
    settings = unpickle_db()
    settings.angle_limit = new_limit
    pickle.dump(settings, open('config.out', 'wb'))
    return ('ok', 200)

@api.route('/set-shutoff-duration', methods=['POST'])
def set_shutoff_duration():
    data_req = request.get_json()
    new_limit = data_req['data']
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
    data_req = request.get_json()

    time = int(data_req['time'])
    if(data_req['is-open'] == 'true'):
        is_open = True 
    else:
        is_open = False

    open_angle = int(data_req['open-angle'])

    put = Log(time, is_open, open_angle)

    curr_logs = unpickle_log()
    curr_logs.append(put)

    pickle.dumps(curr_logs, open('log.out', 'wb'))
