import sys
import random
from scipy.stats import powerlaw
import math
import os

'''
input:
    dir - directory of the dataset
    distributionName - dataset type(zipf/powerlaw/weibull)
    bytePerStr - bytes number per item
    totalNum - total number in dataset
    distinctNum - distinct number in dataset
    parameter1 - for zipf/powerlaw, skewness;
                 for weibull, a parameter in discrete Weibull distribution (0<parameter1<1)
                              (ref http://www.math.wm.edu/~leemis/chart/UDR/PDFs/Discreteweibull.pdf)
    parameter2 - only for weibull, a positive shape parameter

output:
    .dat file - for zipf/powerlaw, 'dir+ distributionName+ parameter1.dat'
                for weibull, 'dir+ distributionName+ parameter1_parameter2.dat'
'''

def powerlaw(N, s):
    res = []
    base = 0.0
    for n in range(1, N + 1):
        t = 1 / (n ** s)
        base += t
        res.append(t)
    return [r / base for r in res]

def weibull(N, p, k):
    res = []
    for n in range(0, N):
        power1 = n ** k
        p1 = (1 - p) ** power1
        power2 = (n + 1) ** k
        p2 = (1 - p) ** power2
        res.append(p1 - p2)
    return res

def random_bytes(byts):
    st = ''
    for j in range(byts):
        st += random.choice(string.printable[:-5])
    b = bytes(st, encoding = 'utf-8')
    return b

def gen_random_strings(len, byts):
    strs = set()
    res = []
    for i in range(len):
        s = os.urandom(byts)
        while s in strs:
            s = os.urandom(byts)
        res.append(s)
        strs.add(s)
    return res

def gen(freqs, byts):
    data = []
    strs = gen_random_strings(len(freqs), byts)
    chs = [i for i in range(len(freqs))]
    while len(chs) != 0:
        p = random.randint(0, len(chs) - 1)
        pos = chs[p]
        data.append(strs[pos])
        freqs[pos] -= 1
        if freqs[pos] == 0:
            del chs[p]
    random.shuffle(data)
    return data

def dataset_write(fp, distriName, byts, tot, dis, pa1, pa2):
    if distriName == 'zipf' or distriName == 'powerlaw':
        props = powerlaw(dis, pa1)
    elif distriName == 'weibull':
        props = weibull(dis, pa1, pa2)

    # if not os.path.exists(fp):
    #     os.mkdir(fp)

    freq = [round(prop * tot)+1 for prop in props]
    dataset = gen(freq, byts)
    #print(len(dataset))

    if distriName == 'zipf' or distriName == 'powerlaw':
        # fpath = fp + distriName + str(pa1) + '.dat'
        fpath = fp
    elif distriName == 'weibull':
        # fpath = fp + distriName + str(pa1) + '_' + str(pa2) + '.dat'
        fpath = fp

    with open(fpath, 'wb') as f:
        for d in dataset:
            f.write(d)
    return fpath

if __name__ == "__main__":
    dir = 'dataset/'
    distributionName = 'weibull'
    bytePerStr = 4
    totalNum = 1000000
    distinctNum = 50000
    parameter1 = 0.3
    parameter2 = 2

    dataset_write(dir, distributionName, bytePerStr, totalNum, distinctNum, parameter1, parameter2)
