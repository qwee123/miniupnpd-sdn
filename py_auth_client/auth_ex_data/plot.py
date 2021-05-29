import matplotlib.pyplot as plt

auth_linux = [0, 0, 0]
auth_sdn = [0, 0, 0]
auth_sdn_auth = [0.15685510635375977, 0.14700889587402344, 0.15595626831054688]

ssdp_linux = [2.1230883598327637, 2.121957778930664, 2.1066653728485107]
ssdp_sdn = [2.1227293014526367, 2.116548538208008, 2.117849349975586]
ssdp_sdn_auth = [2.122299909591675, 2.1186647415161133, 2.115231513977051]

soap_linux = [0.004746675491333008, 0.004601955413818359, 0.0050852298736572266]
soap_sdn = [0.024938344955444336, 0.023906230926513672, 0.025399446487426758]
soap_sdn_auth = [0.030322790145874023, 0.03523969650268555, 0.030979394912719727]

def average(array):
    total = 0
    for value in array:
        total+=value
    
    return total/len(array)

auth = [average(auth_linux), average(auth_sdn), average(auth_sdn_auth)]
ssdp = [average(ssdp_linux), average(ssdp_sdn), average(ssdp_sdn_auth)]
soap = [average(soap_linux), average(soap_sdn), average(soap_sdn_auth)]

plt.bar(['linux', 'sdn', 'sdn_auth'], auth, color='red')

accumulation = auth

plt.bar(['linux', 'sdn', 'sdn_auth'], ssdp, bottom=accumulation, color='green')

for i in range(len(accumulation)):
    accumulation[i] += ssdp[i]

plt.bar(['linux', 'sdn', 'sdn_auth'], soap, bottom=accumulation, color='blue')
plt.yscale('linear')
plt.ylabel('time')

plt.savefig('output.png')