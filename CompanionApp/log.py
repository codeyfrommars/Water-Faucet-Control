class Log():
    def __init__(self, log_time, is_open, open_angle):
        self.log_time = log_time
        self.open = is_open
        if(is_open):
            self.open_angle = open_angle
        else:
            self.open_angle = -1
        