import matplotlib.pyplot as plt
from string import Template
import argparse

parser = argparse.ArgumentParser(description="Plot http connection latency")
parser.add_argument('--normal', '-n', help="Normal Case data filename")
parser.add_argument('--docker', '-d', help="Docker Case data filename")
parser.add_argument('--sdn', '-s', help="SDN Case data filename")
parser.add_argument('--sdn-first', '-f', help="SDN-First Case data filename")
parser.add_argument('--sample-num', '-sn', type=int, help="Number of samples")

namespace = parser.parse_args()
normal_file=namespace.normal
docker_file=namespace.docker
sdn_file=namespace.sdn
sdn_first_file=namespace.sdn_first
sample_num = namespace.sample_num

def parseData(file):
    data_str = file.read()
    data = data_str.split(',')
    for i in range(len(data)-1):
        data[i] = float(data[i])
    return data[:-1]

def average(array):
    total = 0
    for value in array:
        total+=value
    
    return total/len(array)

normal_data = []
docker_data = []
sdn_data = []
sdn_first_data = []

with open(normal_file) as f:
    normal_data = parseData(f)
    f.close()

with open(docker_file) as f:
    docker_data = parseData(f)
    f.close()

with open(sdn_file) as f:
    sdn_data = parseData(f)
    f.close()

with open(sdn_first_file) as f:
    sdn_first_data = parseData(f)
    f.close()

assert len(normal_data) == sample_num
assert len(docker_data) == sample_num
assert len(sdn_data) == sample_num
assert len(sdn_first_data) == sample_num

plot_data = []
plot_data.append(average(normal_data))
plot_data.append(average(docker_data))
plot_data.append(average(sdn_first_data))
plot_data.append(average(sdn_data))

plt.bar(['Normal', 'Nat', 'SDN-First', 'SDN'], plot_data)
plt.title('Http Request Lantency Comparison')
plt.xlabel('Situations')
plt.ylabel('time(ms)')
plt.ylim([0, 15])
plt.savefig('output.png')