from numpy import *
from scipy import signal as ss
import re
import os

def MakeTableStr(table,name,ctype="const double"):
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

def GenerateBLSaw(intervals=4,resolution=2700,fs=48000,fc=15000,beta=8.3,apgain=0.5,apbeta=0.5):
    """ Generate band limited sawtooth wave """
    pts = intervals*resolution+1
    blit = GenerateBlit(intervals,resolution,fs,fc,beta,apgain,apbeta)
    blsawseg = blit.cumsum()
    blsawseg = 2*blsawseg/blsawseg[-1]
    blsawseg[pts/2:] -= 2
    blsawseg /= blsawseg.max()
    naivesaw = ss.sawtooth(arange(pts)*2*pi/pts)
    blsawseg_up = zeros_like(naivesaw)
    
    return blsaw

def GenerateBlit(intervals=10,resolution=100,fs=48000,fc=18300,beta=9.,apgain=0.9,apbeta=0.7):
    """
    Generate a bandlimited dirac delta impulse.

    intervals - number of samples (sampled at fs) to span
    resolution - number of points per sample; e.g. oversampling amount
    fs - sampling frequency
    fc - cutoff
    beta - first window param
    apgain - apodizing window gain (subtracted from the )
    """
    intervals=2*(intervals/2)+1
    pts = resolution*intervals # impulse length in points
    x1 = arange(pts)
    x2 = intervals*2*(x1-(pts)/2+1e-5)/(pts)
    x3 = pi*fc*1.0/fs*x2
    h = array([sin(x)/x if x!=0 else 1.0 for x in x3]) # brickwall filter impulse response
    w = ss.kaiser(pts,beta) # window
    apw = 1-apgain*ss.kaiser(pts,apbeta) # apodization window
    blit = apw*(w*h) # blit

    blit = blit/blit.max() # normalize
    return blit

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

def GenerateSine(n):
    k = arange(n)
    T = 1./n
    c = sin(k*T*2*pi)
    return c

def PitchTable(n,notestart=0,notefinish=128):
    N = linspace(notestart,notefinish,n)
    return 440*2**((N-69.)/12.)

def rewriteAutomatedSection(full_text,replacement_text):
    block_pattern = re.compile(
            " */\\*::(?P<tag>.*?)::\\*/.*?\n *(?P<block>.*)\n */\\*::/(?P=tag)::\\*/",
            re.DOTALL);
    match = block_pattern.search(full_text)
    if not match:
        return full_text
    return re.sub(re.escape(match.groupdict()["block"]), replacement_text, full_text)

def main(pargs):
    v = pargs.verbose

    ss_points = 128
    sinsquaredtable = GenerateSine(ss_points)
    sk_points = 5
    sk_fc = 0.5
    sinckernel = SincKernel(sk_points,sk_fc)
    pitchtable = PitchTable(128,-128,128)
    blsaw = GenerateBLSaw(128)

    key_order = ['size','input_min','input_max', 'isPeriodic']
    tables = {
            'VOSIM_PULSE_COS':dict(data=sinsquaredtable,size=ss_points),
            'PITCH_TABLE':dict(data=pitchtable,size=len(pitchtable),input_min=-1,input_max=1,isPeriodic=False),
            'BL_SAW':dict(data=blsaw,size=len(blsaw))
            }

    tabledata_def = ""
    tabledata_decl = ""
    tableobj_def = ""
    # tableinfostr = '\n'.join(["#define {} {}".format(k,v) for k,v in tableinfo.items()])
    # tableinfostr += '\n'
    for name,struct in tables.items():
        tabledata_def += MakeTableStr(tables[name]['data'],name)
        tabledata_decl += "extern const double {}[{}];\n".format(name,len(tables[name]['data']))

        currargs = []
        for k in key_order:
            if k not in struct:
                continue
            if k=="data":
                continue
            val = struct[k]
            if type(val)==bool:
                val = "true" if val else "false"
            currargs.append("{}".format(val))
        tableargs = "{}, ".format(name)+", ".join(currargs)
        tableobj_currdef = "const LookupTable lut_{}({});\n".format(name.lower(),tableargs);
        tableobj_def += tableobj_currdef
        if v:
            print tableobj_currdef

    if os.path.isfile("table_data.cpp"):
        if v:
            print "Backing up old table_data.cpp..."
        old_table_data = open('table_data.cpp','r').read()
        with open('table_data.cpp.bk','w') as fp:
            fp.write(old_table_data)

    if v:
        print "Writing new table_data.cpp..."
    with open('table_data.cpp','w') as fp:
        fp.write("#include \"tables.h\"\n")
        fp.write("namespace syn {\n")
        fp.write(tabledata_def)
        fp.write("\n}\n")

    if os.path.isfile("tables.h"):
        if v:
            print "Backing up old tables.h..."
        header_text = open('tables.h','r').read()
        with open('tables.h.bk','w') as fp:
            fp.write(header_text)
    if v:
        print "Writing new tables.h..."
    with open('tables.h', 'w') as fp:
        fp.write(rewriteAutomatedSection(header_text, tabledata_decl+'\n'+tableobj_def))

if __name__=="__main__":
    import argparse as ap
    psr = ap.ArgumentParser()
    psr.add_argument("-v","--verbose",action="store_true")
    pargs = psr.parse_args()

    main(pargs)

"""Tools"""
def magspec(signal,fs=None):
    from matplotlib import pyplot as plt
    k = arange(1,len(signal)/2)
    fs = fs or len(signal)
    freqs = k*1./len(signal)*fs
    signalfft = fft.fft(signal)[1:len(signal)/2]
    plt.semilogx( freqs, 20*log10(abs(signalfft)) )

def maggain(signal1,signal2,fs=None):
    from matplotlib import pyplot as plt
    assert len(signal1)==len(signal2)
    k = arange(1,len(signal1)/2)
    fs = fs or len(signal1)
    freqs = k*1./len(signal1)*fs
    signal1fft = fft.fft(signal1)[1:len(signal1)/2]
    signal2fft = fft.fft(signal2)[1:len(signal1)/2]
    plt.semilogx( freqs, 20*(log10(abs(signal1fft))-log10(abs(signal2fft))) )

def slidingwindow(window,signal,window_res,transpose_ratio):
    intervals = int(len(window)/float(window_res))
    norig = len(signal)
    phase_step = transpose_ratio/float(norig)
    nfinal = int(1./phase_step)
    output = zeros(nfinal)
    sphase = 0.0
    for k in xrange(nfinal):
        index = int(sphase*norig)
        offset = window_res-1+(int(sphase)-sphase)*window_res
        print offset
        for i in xrange(intervals):
            output[k] += window[mod(offset+window_res*i,len(window))]*signal[mod(index+i-intervals/2+1,norig)]
        sphase+=phase_step
        if sphase>=1:
            sphase-=1
    return output