from numpy import *
from scipy import signal as ss
import re
import os,sys

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

def GenerateBLSaw(pts,nharmonics=None):
    """ Generate band limited sawtooth wave """
    nfft = pts
    hpts = nharmonics or pts/2+1
    hind = arange(1,hpts)
    freqmags = zeros(pts/2+1,dtype=complex128)
    freqmags[hind] = 1j*1./hind
    blsaw = fft.irfft(freqmags,n=nfft)
    return blsaw/blsaw.max()

def GenerateBlit(intervals=10,resolution=100,fs=48000,fc=18300,beta=9.,apgain=0.9,apbeta=0.7):
    """
    Generate a bandlimited dirac delta impulse.

    intervals - number of samples (sampled at fs) to span
    resolution - number of points per sample; e.g. oversampling amount
    fs - sampling frequency
    fc - cutoff
    beta - first window param
    apgain - apodizing window gain
    """
    Ts = 1./fs
    intervals=2*(int(intervals)/2)+1 # make intervals odd
    pts = resolution*intervals # impulse length in points
    ind = arange(pts)*1./resolution-intervals/2 # pts points spread across [-intervals/2,intervals/2)
    x = fc*Ts*ind # pts points spread across fc/fs*[-intervals/2, intervals/2)
    h = sinc(x)
    w = ss.kaiser(pts,beta) # window
    apw = 1-apgain*ss.kaiser(pts,apbeta) # apodization window
    blit = (apw*w)*h # blit

    blit = blit/2./blit.max() # normalize
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

    ss_points = 1024
    sintable = GenerateSine(ss_points)
    pitchtable = PitchTable(256,-128,128)
    blsaw = GenerateBLSaw(256,16)

    key_order = ['size','input_min','input_max', 'isPeriodic']
    tables = {
            'SIN':dict(data=sintable,size=ss_points),
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
def magspec(signal, pts=None, fs=None, logscale=True):
    from matplotlib import pyplot as plt
    pts = pts or len(signal)
    k = arange(1,pts/2)
    fs = fs or 1.
    freqs = linfreqs = k*1./pts*fs
    if(logscale):
        freqs = log2(freqs)
    signalfft = fft.fft(signal,n=pts)[1:pts/2]/pts
    if(all(signalfft)):
        mag = 20*log10(abs(signalfft))
        ylabel = "dB"
    else:
        mag = abs(signalfft)**2
        ylabel = "Amp"
    f = plt.gcf()
    ax = plt.gca()
    if logscale:
        ax.semilogx( linfreqs, mag, basex=2)
    else:
        ax.plot( linfreqs, mag )
    ax.set_xlabel("Hz")
    ax.set_ylabel(ylabel)
    ax.grid(True,which='both')
    f.show()

def maggain(signal1,signal2,pts=None,fs=None,logscale=True):
    from matplotlib import pyplot as plt
    fftlength = pts or max(len(signal1),len(signal2))
    k = arange(1,fftlength/2)
    fs = fs or 1.
    linfreqs = freqs = k*1./fftlength*fs
    signal1fft = fft.fft(signal1,n=fftlength)[1:fftlength/2]/fftlength
    signal2fft = fft.fft(signal2,n=fftlength)[1:fftlength/2]/fftlength
    if(all(signal1fft) and all(signal2fft)):
        mag1 = 20*log10(abs(signal1fft))
        mag2 = 20*log10(abs(signal2fft))
        gain = mag2-mag1
        ylabel = "dB"
    else:
        mag1 = abs(signal1fft)**2
        mag2 = abs(signal2fft)**2
        gain = mag2-mag1
        ylabel = "Amp"
    f = plt.gcf()
    ax = plt.gca()
    if logscale:
        ax.semilogx( linfreqs, gain, basex=2)
    else:
        ax.plot( linfreqs, gain )
    ax.set_xlabel("Hz")
    ax.set_ylabel(ylabel)
    ax.grid(True,which='both')
    f.show()

def lerp(a,b,frac):
    return (b-a)*frac + a

def slidewindow(signal,window,resolution,fc=1.0):
    M = arange(len(signal))
    winlen = len(window)/resolution/2
    newsig = zeros(len(signal))
    scale = 1.0
    for m in M:
        for n in xrange(m-winlen,m+winlen+1):
            k = mod(n,len(signal))
            filtphase = resolution*(m-n+winlen)
            tableindex = int(fc*filtphase)
            tableindex_frac = fc*filtphase - tableindex
            tableindex_nxt = mod(tableindex+1,len(window))
            newsig[m] += signal[k]*lerp(window[tableindex],window[tableindex_nxt],tableindex_frac)
    return newsig

def sinclowpass(signal,fc,winlen=10):
    newsignal = zeros(len(signal))
    M = arange(len(signal))
    N = arange(len(signal))
    # for m in M:
        # for n in xrange(N):
            # newsignal[m] += signal[n]*sinc(fc*(m-n))
        # newsignal[m]*=2*fc
    fc = fc
    sincfir = sinc(fc*arange(-winlen,winlen+1))
    apwin = genApodWindow(len(sincfir),9.,0.9,0.7)
    sincfir *= apwin
    sincfir /= sincfir.max()
    for m in M:
        for n in xrange(m-winlen,m+winlen+1):
            k = mod(n,len(signal))
            newsignal[m] += signal[k]*sincfir[m-n+winlen]
    return newsignal*fc


def genApodWindow(pts,beta,apgain,apbeta):
    w = ss.kaiser(pts,beta) # window
    apw = 1-apgain*ss.kaiser(pts,apbeta) # apodization window
    W = w*apw
    return W
