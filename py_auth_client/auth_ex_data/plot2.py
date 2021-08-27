import matplotlib.pyplot as plt
from string import Template
import math

linux_auth = []
linux_ssdp = []
linux_soap = []
linux_s_pr = []
linux_s_ver = []
linux_s_soap = []

sdn_auth = []
sdn_ssdp = []
sdn_soap = []
sdn_s_pr = []
sdn_s_ver = []
sdn_s_soap = []

sdnauth_auth = []
sdnauth_ssdp = []
sdnauth_soap = []
sdnauth_s_pr = []
sdnauth_s_ver = []
sdnauth_s_soap = []

def parseLinuxData(file):
    line = file.readline()
    while line:
        pair = line.split(':', 1)

        if pair[0] == 'Time to Prepare Request(ms)':
            linux_s_pr.append(float(pair[1]))
        elif pair[0] == 'auth':
            linux_auth.append(float(pair[1])*1000) #sec -> msec
        elif pair[0] == 'discover&get description':
            linux_ssdp.append(float(pair[1])*1000) #sec -> msec
        elif pair[0] == 'soap':
            linux_soap.append(float(pair[1])*1000) #sec -> msec
        elif pair[0] == 'Verification Interval':
            linux_s_ver.append(float(pair[1])/1000) #usec -> msec
        elif pair[0] == 'SOAP Interval':
            linux_s_soap.append(float(pair[1])/1000) #usec -> msec
        line = file.readline()

def parseSDNData(file):
    line = file.readline()
    while line:
        pair = line.split(':', 1)

        if pair[0] == 'Time to Prepare Request(ms)':
            sdn_s_pr.append(float(pair[1]))
        elif pair[0] == 'auth':
            sdn_auth.append(float(pair[1])*1000) #sec -> msec
        elif pair[0] == 'discover&get description':
            sdn_ssdp.append(float(pair[1])*1000) #sec -> msec
        elif pair[0] == 'soap':
            sdn_soap.append(float(pair[1])*1000) #sec -> msec
        elif pair[0] == 'Verification Interval':
            sdn_s_ver.append(float(pair[1])/1000) #usec -> msec
        elif pair[0] == 'SOAP Interval':
            sdn_s_soap.append(float(pair[1])/1000) #usec -> msec
        line = file.readline()

def parseSDNAuthData(file):
    line = file.readline()
    while line:
        pair = line.split(':', 1)

        if pair[0] == 'Time to Prepare Request(ms)':
            sdnauth_s_pr.append(float(pair[1]))
        elif pair[0] == 'auth':
            sdnauth_auth.append(float(pair[1])*1000) #sec -> msec
        elif pair[0] == 'discover&get description':
            sdnauth_ssdp.append(float(pair[1])*1000) #sec -> msec
        elif pair[0] == 'soap':
            sdnauth_soap.append(float(pair[1])*1000) #sec -> msec
        elif pair[0] == 'Verification Interval':
            sdnauth_s_ver.append(float(pair[1])/1000) #usec -> msec
        elif pair[0] == 'SOAP Interval':
            sdnauth_s_soap.append(float(pair[1])/1000) #usec -> msec
        line = file.readline()

def average(array):
    total = 0
    for value in array:
        total+=value
    
    return total/len(array)

def std_devia(array, avg):
    squared_total = 0
    for value in array:
        squared_total += math.pow((value-avg), 2)

    return math.sqrt(squared_total/(len(array)-1))

with open('linux.txt') as f:
    parseLinuxData(f)
    f.close()

with open('sdn.txt') as f:
    parseSDNData(f)
    f.close()

with open('sdnauth.txt') as f:
    parseSDNAuthData(f)
    f.close()

linux_auth_avg = average(linux_auth)
linux_ssdp_avg = average(linux_ssdp)
linux_soap_avg = average(linux_soap)
linux_s_pr_avg = average(linux_s_pr)
linux_s_ver_avg = average(linux_s_ver)
linux_s_soap_avg = average(linux_s_soap)
linux_auth_devia = std_devia(linux_auth, linux_auth_avg)
linux_ssdp_devia = std_devia(linux_ssdp, linux_ssdp_avg)
linux_soap_devia = std_devia(linux_soap, linux_soap_avg)
linux_s_pr_devia = std_devia(linux_s_pr, linux_s_pr_avg)
linux_s_ver_devia = std_devia(linux_s_ver, linux_s_ver_avg)
linux_s_soap_devia = std_devia(linux_s_soap, linux_s_soap_avg)

sdn_auth_avg = average(sdn_auth)
sdn_ssdp_avg = average(sdn_ssdp)
sdn_soap_avg = average(sdn_soap)
sdn_s_pr_avg = average(sdn_s_pr)
sdn_s_ver_avg = average(sdn_s_ver)
sdn_s_soap_avg = average(sdn_s_soap)
sdn_auth_devia = std_devia(sdn_auth, sdn_auth_avg)
sdn_ssdp_devia = std_devia(sdn_ssdp, sdn_ssdp_avg)
sdn_soap_devia = std_devia(sdn_soap, sdn_soap_avg)
sdn_s_pr_devia = std_devia(sdn_s_pr, sdn_s_pr_avg)
sdn_s_ver_devia = std_devia(sdn_s_ver, sdn_s_ver_avg)
sdn_s_soap_devia = std_devia(sdn_s_soap, sdn_s_soap_avg)

sdnauth_auth_avg = average(sdnauth_auth)
sdnauth_ssdp_avg = average(sdnauth_ssdp)
sdnauth_soap_avg = average(sdnauth_soap)
sdnauth_s_pr_avg = average(sdnauth_s_pr)
sdnauth_s_ver_avg = average(sdnauth_s_ver)
sdnauth_s_soap_avg = average(sdnauth_s_soap)
sdnauth_auth_devia = std_devia(sdnauth_auth, sdnauth_auth_avg)
sdnauth_ssdp_devia = std_devia(sdnauth_ssdp, sdnauth_ssdp_avg)
sdnauth_soap_devia = std_devia(sdnauth_soap, sdnauth_soap_avg)
sdnauth_s_pr_devia = std_devia(sdnauth_s_pr, sdnauth_s_pr_avg)
sdnauth_s_ver_devia = std_devia(sdnauth_s_ver, sdnauth_s_ver_avg)
sdnauth_s_soap_devia = std_devia(sdnauth_s_soap, sdnauth_s_soap_avg)

template='''
$situation
auth: $auth($auth_devia)
ssdp: $ssdp($ssdp_devia)
soap: $soap($soap_devia)
s_prepare_request: $s_pr($s_pr_devia)
s_verify_auth: $s_ver($s_ver_devia)
s_soap: $s_soap($s_soap_devia)
'''

t = Template(template)
print(t.substitute({
    'situation': 'linux',
    'auth': linux_auth_avg,
    'ssdp': linux_ssdp_avg,
    'soap': linux_soap_avg,
    's_pr': linux_s_pr_avg,
    's_ver': linux_s_ver_avg,
    's_soap': linux_s_soap_avg,
    'auth_devia': linux_auth_devia,
    'ssdp_devia': linux_ssdp_devia,
    'soap_devia': linux_soap_devia,
    's_pr_devia': linux_s_pr_devia,
    's_ver_devia': linux_s_ver_devia,
    's_soap_devia': linux_s_soap_devia
}))

print(t.substitute({
    'situation': 'sdn',
    'auth': sdn_auth_avg,
    'ssdp': sdn_ssdp_avg,
    'soap': sdn_soap_avg,
    's_pr': sdn_s_pr_avg,
    's_ver': sdn_s_ver_avg,
    's_soap': sdn_s_soap_avg,
    'auth_devia': sdn_auth_devia,
    'ssdp_devia': sdn_ssdp_devia,
    'soap_devia': sdn_soap_devia,
    's_pr_devia': sdn_s_pr_devia,
    's_ver_devia': sdn_s_ver_devia,
    's_soap_devia': sdn_s_soap_devia
}))

print(t.substitute({
    'situation': 'sdnauth',
    'auth': sdnauth_auth_avg,
    'ssdp': sdnauth_ssdp_avg,
    'soap': sdnauth_soap_avg,
    's_pr': sdnauth_s_pr_avg,
    's_ver': sdnauth_s_ver_avg,
    's_soap': sdnauth_s_soap_avg,
    'auth_devia': sdnauth_auth_devia,
    'ssdp_devia': sdnauth_ssdp_devia,
    'soap_devia': sdnauth_soap_devia,
    's_pr_devia': sdnauth_s_pr_devia,
    's_ver_devia': sdnauth_s_ver_devia,
    's_soap_devia': sdnauth_s_soap_devia
}))

pr = [linux_s_pr_avg, sdn_s_pr_avg, sdnauth_s_pr_avg]
ver = [linux_s_ver_avg, sdn_s_ver_avg, sdnauth_s_ver_avg]
soap = [linux_s_soap_avg, sdn_s_soap_avg, sdnauth_s_soap_avg]

plt.bar(['linux', 'sdn', 'sdn_auth'], pr, color='red', label='build request')

accumulation = pr

plt.bar(['linux', 'sdn', 'sdn_auth'], ver, bottom=accumulation, color='green', label='verify token')

for i in range(len(accumulation)):
    accumulation[i] += ver[i]

plt.bar(['linux', 'sdn', 'sdn_auth'], soap, bottom=accumulation, color='blue', label='set DNAT')
plt.title('Process Time in Each Situation')
plt.yscale('linear')
plt.ylabel('time(ms)')
plt.legend(loc='upper left')

plt.savefig('output.png')