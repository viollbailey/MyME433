import matplotlib.pyplot as plt
import numpy as np
import csv

tA = []
tB = []
tC = []
tD = []

dataA = []
dataB = []
dataC = []
dataD = []

with open('sigA.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        tA.append(float(row[0])) # leftmost column
        dataA.append(float(row[1])) # second column

with open('sigB.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        tB.append(float(row[0])) # leftmost column
        dataB.append(float(row[1])) # second column

with open('sigC.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        tC.append(float(row[0])) # leftmost column
        dataC.append(float(row[1])) # second column

with open('sigD.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        tD.append(float(row[0])) # leftmost column
        dataD.append(float(row[1])) # second column

def my_fft(t, data, title):
    Ts = t[1] - t[0]; # sampling interval
    ts = np.arange(0,t[-1],Ts) # time vector
    y = data # the data to make the fft from
    n = len(y) # length of the signal
    k = np.arange(n)
    T = n * Ts
    frq = k/T # two sides frequency range
    frq = frq[range(int(n/2))] # one side frequency range
    Y = np.fft.fft(y)/n # fft computing and normalization
    Y = Y[range(int(n/2))]

    ax1.plot(t,y)
    ax1.title.set_text(title)
    ax1.set_xlabel('Time')
    ax1.set_ylabel('Amplitude')
    ax2.loglog(frq,abs(Y)) # plotting the fft
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')

# Initial plots
# my_fft(tA, dataA, "SigA")
# plt.show()
# # my_fft(tB, dataB, "SigB")
# plt.show()

# # my_fft(tC, dataC, "SigC")
# plt.show()

# # my_fft(tD, dataD, "SigD")
# plt.show()


# MAF data generation 
# I'm just going to be using copy and paste for this since it's easier than
# working out how to get stuff out of functions in python

def MAF(X, t, data, title):
    data_Extended = [0] * X
    data_Extended = data_Extended + data
    data_MAF = []
    for i in range(len(t)):
        # read the rows one by one
        MAF = 0
        for j in range(X):
            MAF = MAF + data_Extended[i + j] # second column
        MAF = MAF / X
        data_MAF.append(MAF)
    my_fft(t,data, title)
    my_fft(t,data_MAF, title)

# fig, (ax1, ax2) = plt.subplots(2, 1)
# MAF(2000, tA, dataA, "SigA MAF X = 200")
# plt.show()

# fig, (ax1, ax2) = plt.subplots(2, 1)
# MAF(200, tB, dataB, "SigB MAF X = 200")
# plt.show()

# fig, (ax1, ax2) = plt.subplots(2, 1)
# MAF(2000, tC, dataC, "SigC MAF X = 200")
# plt.show()

# fig, (ax1, ax2) = plt.subplots(2, 1)
# MAF(200, tD, dataD, "SigD MAF X = 200")
# plt.show()


def IIR(A, B, t, data, title):

    data_IIR = []
    ave = 0
    for i in range(len(t)):
        ave = A * ave + B * data[i] 
        data_IIR.append(ave)
    my_fft(t,data, title)
    my_fft(t,data_IIR, title)


fig, (ax1, ax2) = plt.subplots(2, 1)
IIR(.999, .001, tA, dataA, "SigA IIR A = .999, B = .001")
plt.show()

fig, (ax1, ax2) = plt.subplots(2, 1)
IIR(.999, .001, tB, dataB, "SigB IIR A = .999, B = .001")
plt.show()

fig, (ax1, ax2) = plt.subplots(2, 1)
IIR(.999, .001, tC, dataC, "SigC IIR A = .999, B = .001")
plt.show()

fig, (ax1, ax2) = plt.subplots(2, 1)
IIR(.999, .001, tD, dataD, "SigD IIR A = .999, B = .001")
plt.show()