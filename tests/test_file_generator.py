import numpy as np
import glob, os, os.path


class Ensemble:
    
    def __init__(self, name, mean, sd, hz) -> None:
        self.name = name
        self.mean = mean
        self.sd = sd
        self.interval = int(1/hz * 1000)
        self.duration = mean + (3 * sd)
        self.max_delay = 0
    
class Input:
    def __init__(self, name, ensembles) -> None:
        
        self.directory = "tests/no_check_inputs/" + str(name) + ".txt"
        self.ensembles = ensembles
        minutes = 0.25
        self.end = minutes * 60000
    def write(self):
        with open(self.directory, 'w') as f:
            f.write("START\n")
            f.write("0\n")
            f.write("END\n")
            f.write(str(int(self.end)) + "\n")
            f.write("ENSEMBLES\n")
            for ensemble in self.ensembles:
                if int(ensemble.interval) == 0:
                    ensemble.interval = 1
                if int(ensemble.duration) == 0:
                    ensemble.duration = 1
                f.write(str(ensemble.name) + "|" + str(int(ensemble.interval))+ "|" + str(int(ensemble.duration))+ "|" +  str(int(ensemble.max_delay)) +  "\n")
            delays = self.generate_delays()
            f.write("DELAYS\n")
            for delay in delays:
                f.write(delay)
                
                
    def generate_delays(self):
        delays = []
        for ensemble in self.ensembles:
            num_delays = int(self.end / ensemble.interval)
            delay_list = np.random.normal(ensemble.mean, 1.2 * ensemble.sd,num_delays)
            i = 0
            for delay in delay_list:
                delays.append(ensemble.name + "|" + str(i) +"|"+ str(int(delay)) + "\n")
                i+=1
        return delays
            
            

if __name__ == "__main__":
    filelist = glob.glob(os.path.join("tests/no_check_inputs/", "test*.txt"))
    for f in filelist:
        os.remove(f)
    file_num = 0
    ensembles = {}
    gyro_sweep = [30,300]
    mag_sweep = [30,300]
    input = None
    ensembles["Temperature"] = Ensemble("Temperature",0.873,1,1)
    ensembles["GPS"] = Ensemble("GPS",0.1443,1,1)
    ensembles["Wet/Dry Sensor"] = Ensemble("Wet/Dry Sensor",1.02,1,1)
    step = 5
    for gyro_interval in range(gyro_sweep[0],gyro_sweep[1]+1,step):
        ensembles["Gyrometer"] = Ensemble("Gyrometer",5.209,1, gyro_interval)
        for mag_interval in range(mag_sweep[0],mag_sweep[1]+1,step):
            ensembles["Magnetometer"] = Ensemble("Magnetometer",5.212,1, mag_interval)
            input = Input("test" + str(file_num), ensembles.values())
            file_num += 1
            input.generate_delays()
            input.write()
            print(f'{file_num}/{(250/step)**2}' + "\r")


    
'''
START #only one number
0
END #only one number
10000
ENSEMBLES #taskname|interval|duration or taskname|interval|duration|maxDelay
A|2000|200
B|2000|400|1000
C|500|100|100
DELAYS #0 based indexing: "taskname|iteration|delay"
B|1|1000
RESETS # "taskname|iteration" check when task is skipped or delayed too much
C|3
'''