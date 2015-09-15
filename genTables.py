from numpy import *

def MakeTableStr(table,name,ctype="double"):
    rows = int(sqrt(table.size))
    cols = int(ceil(sqrt(table.size)))
    showstr = "{} {}[{}] = {{\n".format(ctype,name,table.size)
    cellstr = "{:.18f}"
    ind = 0
    for i in xrange(rows):
        if i!=0:
            showstr+='\n'
        for j in xrange(cols):
            showstr+=cellstr.format(table[ind])
            showstr+=','
            ind+=1
            if ind>=table.size:
                break
        if ind>table.size:
            break
    showstr+="\n};\n"
    return showstr

def RealCepstrum(n,signal):
    #realTime = zeros(n)
    #imagTime = zeros(n)

    #for i in xrange(n):
        #realTime[i] = signal[i]

    #realFreq,imagFreq = DFT(n,realTime,imagTime)
    #for i in xrange(n):
        #realFreq[i] = log(cabs(realFreq[i],imagFreq[i]))
        #imagFreq[i] = 0

    #realTime,imagTime = InverseDFT(n,realFreq,imagFreq)

    freq = fft.fft(signal)
    freq = log(abs(freq))
    realTime = real(fft.ifft(freq))

    realCepstrum = realTime
    return realCepstrum

def MinimumPhase(n,realCepstrum):
    nd2 = n/2
    realTime = zeros(n)
    imagTime = zeros(n)
    realFreq = zeros(n)
    imagFreq = zeros(n)
    realTime[0] = realCepstrum[0]
    for i in xrange(1,nd2):
        realTime[i] = 2*realCepstrum[i]
    if ((n%2)==0):
        realTime[nd2] = realCepstrum[nd2]

    #realFreq,imagFreq = DFT(n,realTime,imagTime)
    #for i in xrange(n):
        #realFreq[i],imagFreq[i] = cexp(realFreq[i],imagFreq[i])
    #realTime,imagTime = InverseDFT(n,realFreq,imagFreq)
    freq = exp(fft.fft(realTime))
    realTime = real(fft.ifft(freq))


    minimumPhase = realTime
    return minimumPhase

def GenerateMinBLEP(nZeroCrossings, nOverSampling):
    n = (nZeroCrossings * 2 * nOverSampling) + 1
    buffer1 = zeros(n)
    buffer2 = zeros(n)

    a = -float(nZeroCrossings)
    b = float(nZeroCrossings)
    #for i in xrange(n):
        #r = float(i)/(n-1)
        #buffer1[i] = SINC( a + (r * (b-a)) )
    x = (a + arange(n)/float(n-1)*(b-a))
    buffer1 = sinc(x)

    #buffer2 = blackman(n)
    #for i in xrange(n):
        #buffer1[i] *= buffer2[i]
    buffer1 *= blackman(n)

    buffer2 = RealCepstrum(n,buffer1)
    buffer1 = MinimumPhase(n,buffer2)

    #minBLEP = zeros(n)
    #a = 0
    #for i in xrange(n):
        #a += buffer1[i]
        #minBLEP[i] = a
    minBLEP = buffer1.cumsum()

    #a = 1./minBLEP[n-1]
    #for i in xrange(n):
        #minBLEP[i] *= a
    minBLEP/=minBLEP[n-1]

    return minBLEP

def GenerateSinSquared(n):
    t = linspace(0,1,n)
    c = 0.5*(1-cos(t*2*pi))
    return c

def SincKernel(M,fc):
    N = arange(M+1)
    window = 0.42-0.5*cos(2*pi*N/M)+0.8*cos(4*pi*N/M)
    wsinc=array([sin(2*pi*fc*(i-M/2))/(i-M/2) if i!=M/2 else 2*pi*fc for i in N])
    return wsinc*window

def main():
    mb_points = 16
    mb_oversamp = 256
    minblep = GenerateMinBLEP(mb_points,mb_oversamp)
    ss_points = 65536
    sinsquaredtable = GenerateSinSquared(ss_points)
    sk_points = 50
    sk_fc = 0.49
    sinckernel = SincKernel(sk_points,sk_fc)

    tableinfo = {
            'BLEPBUF': int(float(len(minblep))/mb_oversamp),
            'BLEPSIZE': len(minblep),
            'BLEPOS': mb_oversamp,
            'VOSIM_PULSE_COS_SIZE': ss_points,
            'SINC_KERNEL_SIZE': sk_points+1
            }
    tablenames = {
            'MINBLEP':minblep,
            'VOSIM_PULSE_COS':sinsquaredtable,
            'SINC_KERNEL':sinckernel,
            }

    tablestr = ""
    tableinfostr = '\n'.join(["#define {} {}".format(k,v) for k,v in tableinfo.items()])
    tableinfostr += '\n'
    for k in tablenames:
        tablestr += MakeTableStr(tablenames[k],k)
        tableinfostr += "extern double {}[{}];\n".format(k,len(tablenames[k]))

    with open('tables.cpp','w') as fp:
        fp.write(tablestr)
    with open('tables.h', 'w') as fp:
        fp.write(tableinfostr)

if __name__=="__main__":
    main()
