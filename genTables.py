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

def DSF(f0, fm, a, N, pts=None):
    """
    Digital summation formula.

    @returns $sum_{n=1}^{N} a^n sin(2\pi f0 t + 2\pi n fm t)$

    """
    pts = pts or 2*N
    # phase_shift = 2*pi*f0*arange(pts)*1.0/(pts)
    phase_shift = 2*pi*f0
    phases = 2*pi*fm*arange(pts)*1.0/(pts)
    numerator = sin(phase_shift)-a*sin(phase_shift-phases)-a**(N+1)*(sin(phase_shift+(N+1)*phases)-a*sin(phase_shift+N*phases))
    denominator = 1 + a**2 - 2*a*cos(phases)
    result = numerator / denominator
    result[where(denominator)==0]=1.0
    return result/result.max()

def GenerateBlit(pts,nharmonics=None):
    """
    Generate a band-limited impulse train using the FFT

    """
    pts = (pts*2)/2+1
    nharmonics = nharmonics or pts/2
    blit_spectrum = zeros(pts,dtype=complex128)
    blit_spectrum[:nharmonics] = 1.0*1j
    blit = fft.fftshift(fft.irfft(blit_spectrum))
    blit = blit/blit.max()
    return blit

def GenerateBLSaw(pts,nharmonics=None):
    """ Generate band limited sawtooth wave """
    nfft = pts
    hpts = nharmonics or pts/2+1
    hind = arange(1,hpts)
    freqmags = zeros(pts/2+1,dtype=complex128)
    freqmags[hind] = 1j*1./hind
    blsaw = fft.fftshift(fft.irfft(freqmags,n=nfft))
    return blsaw/blsaw.max()

def GenerateBlimp(intervals=10,resolution=2048,fs=48000,fc=20000,beta=9.,apgain=0.9,apbeta=0.7,ret_half=True):
    """
    Generate a bandlimited dirac delta impulse.

    intervals - number of samples (sampled at fs) to span
    resolution - number of points per sample; e.g. oversampling amount
    fs - sampling frequency
    fc - cutoff
    beta - first window param
    apgain - apodizing window gain
    """
    intervals=2*(int(intervals)/2)+1 # make intervals odd
    pts = resolution*intervals+1 # impulse length in points
    ind = intervals*2*(arange(pts) - (pts-1)/2.)/(pts-1.) # pts points spread across [-intervals/2,intervals/2)
    x = fc*1./fs*ind # pts points spread across fc/fs*[-intervals/2, intervals/2)
    h = sinc(x)

    w = ss.kaiser(pts,beta) # window
    apw = 1-apgain*ss.kaiser(pts,apbeta) # apodization window
    window = w*apw
    blimp = window*h # blimp
    # blimp /= blimp.max()
    if ret_half:
        return blimp[len(blimp)/2:]
    else:
        return blimp

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

def InvPitchTable(pitchtable):
    invpitchlen = ceil(len(pitchtable)*1./min(diff(pitchtable)))
    invpitchtable = arange(invpitchlen)[1:]*1./invpitchlen*max(pitchtable)
    return log2(invpitchtable)

def rewriteAutomatedSection(full_text,tag,replacement_text):
    """
    Finds a portion of text surrounded by:

        /*::tag::*/
        ...
        /*::/tag::*/

    And replaces it with the provided replacement text.

    """

    block_pattern = re.compile(
            "\n*(?P<spacing>\\s*?)/\\*::{0:}::\\*/.*?\\s*(?P<block>.*)\n?\\s*/\\*::/{0:}::\\*/".format(tag),
            re.DOTALL);
    match = block_pattern.search(full_text)
    if not match:
        return full_text
    replacement_text=re.sub("\n\\s*","\n"+match.groupdict()['spacing'],replacement_text)
    result = full_text[:match.start("block")] + replacement_text + full_text[match.end("block"):]
    return result

def make_symmetric_r(right_half):
    left_half = list(reversed(right_half))
    return hstack([left_half[:-1],right_half])

def make_symmetric_l(left_half):
    right_half = list(reversed(left_half))
    return hstack([left_half[:-1],right_half])

"""Tools"""
def magspec(signal, pts=None, fs=None, logscale=True, **kwargs):
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
        ax.semilogx( linfreqs, mag, basex=10, **kwargs)
    else:
        ax.plot( linfreqs, mag, **kwargs)
    ax.set_xlabel("Hz")
    ax.set_ylabel(ylabel)
    ax.grid(True,which='both')
    f.show()

def maggain(signal1,signal2,pts=None,fs=None,logscale=True,**kwargs):
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
        ax.semilogx( linfreqs, gain, basex=10, **kwargs)
    else:
        ax.plot( linfreqs, gain, **kwargs )
    ax.set_xlabel("Hz")
    ax.set_ylabel(ylabel)
    ax.grid(True,which='both')
    f.show()

def unitplot(*sigs,**kwargs):
    """
    plots multiple signals on the same timescale

    """
    from matplotlib import pyplot as plt
    f = plt.gcf()
    ax = plt.gca()
    pltsigs = []
    for sig in sigs:
        pltsigs.append( linspace(0,1,len(sig)) )
        pltsigs.append( sig )
    ax.plot(*pltsigs,**kwargs)
    ax.grid(True,which="both")
    f.show()

def lerp(a,b,frac):
    return (b-a)*frac + a

def upsample(signal,amt,filt,filt_res=1):
    """
    signal - signal to upsample
    amt - integer amount to upsample
    filt - interpolation kernel
    filt_res - interpolation kernel oversampling factor

    """
    siglen = len(signal)
    newsiglen = siglen * amt
    newsig = zeros(newsiglen)
    halffiltlen = len(filt)/filt_res/2
    # y[ j + iL ] = sum_{k=0}^{K} x[ i - k ] * h[ j + k*L]
    # i = index of original sample
    # j = index of interpolated sample
    # L = upsample amount
    for i in xrange(siglen):
        for j in xrange(amt):
            newsigind = j + i*amt
            for k in xrange(-halffiltlen,halffiltlen+1):
                filtind = filt_res*(j + k*amt + halffiltlen)
                sigind = clip( i - k, 0, siglen-1 )
                if filtind>=0 and filtind < 2*halffiltlen*filt_res:
                    newsig[ newsigind ] += signal[ sigind ] * filt[ filtind ]
    return newsig

def resample(signal, new_table_size, filt, filt_res, numperiods=1, isperiodic=True, useinterp=True):
    """
    signal - signal to resample
    ratio - ratio of new sample rate to old sample rate
    filt - the symmetric filter kernel to use (should contain only one "wing")
    filt_res - oversampling factor of filter kernel

    """
    filt_len = len(filt)/filt_res
    siglen = len(signal)
    ratio = new_table_size*1.0/siglen
    newsiglen = int(new_table_size*numperiods)
    newsig = zeros(newsiglen)
    phases = zeros(newsiglen)
    phase_step = 1.0/new_table_size

    filt_step = filt_res
    if ratio < 1:
        filt_step *= ratio

    phase = 0
    new_index = 0
    max_taps_used = 0
    currperiod = 0
    while new_index < newsiglen:
        phases[new_index] = phase
        index = int(phase*siglen)
        frac_index = phase*siglen - index
        offset = frac_index * filt_step
        filt_phase = offset
        i = 0
        num_taps_used = 0
        filt_sum = 0
        while filt_phase < len(filt):
            if useinterp:
                next_filt_phase = clip(int(filt_phase)+1,0,len(filt)-1)
                frac_filt_phase = filt_phase - int(filt_phase)
                filt_sample = lerp(filt[int(filt_phase)],filt[next_filt_phase],frac_filt_phase)
            else:
                filt_sample = filt[int(filt_phase)]
            sig_index = index - i
            if isperiodic:
                sig_index = mod(sig_index,siglen)
                newsig[new_index] += filt_sample * signal[sig_index]
            elif sig_index >= 0 and sig_index < siglen:
                newsig[new_index] += filt_sample * signal[sig_index]
            filt_phase += filt_step
            filt_sum += filt_sample
            i+=1
        num_taps_used += i-1
        filt_phase = filt_step - offset
        i = 0
        while filt_phase < len(filt):
            if useinterp:
                next_filt_phase = clip(int(filt_phase)+1,0,len(filt)-1)
                frac_filt_phase = filt_phase - int(filt_phase)
                filt_sample = lerp(filt[int(filt_phase)],filt[next_filt_phase],frac_filt_phase)
            else:
                filt_sample = filt[int(filt_phase)]
            sig_index = index + 1 + i
            if isperiodic:
                sig_index = mod(sig_index,siglen)
                newsig[new_index] += filt_sample * signal[sig_index]
            elif sig_index >= 0 and sig_index < siglen:
                newsig[new_index] += filt_sample * signal[sig_index]
            filt_phase += filt_step
            filt_sum += filt_sample
            i+=1
        newsig[new_index]*=1./filt_sum
        num_taps_used += i-1
        if num_taps_used > max_taps_used:
            max_taps_used = num_taps_used
        phase += phase_step
        new_index+=1
        if phase >= 1:
            phase -= 1
            currperiod+=1
    # newsig *= min(1,siglen*1./newsiglen)
    return newsig

def sinclowpass(signal,fc,winlen=10):
    newsignal = zeros(len(signal))
    M = arange(len(signal))
    # for m in M:
        # for n in xrange(N):
            # newsignal[m] += signal[n]*sinc(fc*(m-n))
        # newsignal[m]*=2*fc
    fc = fc
    sincfir = sinc(fc*arange(-winlen+1,winlen+1))
    apwin = genApodWindow(len(sincfir),9.,0.9,0.7)
    sincfir *= apwin
    sincfir /= sincfir.max()
    for m in M:
        for n in xrange(m-winlen+1,m+winlen+1):
            k = clip(n,0,len(signal)-1)
            newsignal[m] += signal[k]*sincfir[m-n+winlen]
    return newsignal*fc

def genApodWindow(pts,beta,apgain,apbeta):
    w = ss.kaiser(pts,beta) # window
    apw = 1-apgain*ss.kaiser(pts,apbeta) # apodization window
    W = w*apw
    return W

def normalize_power(signal):
    signal_norm = signal/sqrt(sum(signal**2))*sqrt(0.5)
    return signal_norm

def main(pargs):
    v = pargs.verbose
    prefilter_lhalf=[0.0028,-0.0119,0.0322,-0.0709,0.1375,-0.2544,0.4385,-0.6334,1.7224]
    prefilter=make_symmetric_l(prefilter_lhalf)

    offline_intervals = 512
    offline_res = 128

    online_intervals = 10
    online_res = 8192

    sintable = GenerateSine(8192)
    pitchtable = PitchTable(2048,0,128)

    blsaw = GenerateBLSaw(2048,1024)
    blsaw /= blsaw.max()

    blimp_online = GenerateBlimp(online_intervals,online_res,fc=20e3)
    blimp_offline = GenerateBlimp(offline_intervals,offline_res,fc=20e3)

    blimp_online = convolve( prefilter, blimp_online, 'same' ) # apply gain to high frequencies
    blimp_offline = convolve( prefilter, blimp_offline, 'same' ) # apply gain to high frequencies

    blimp_online /= blimp_online.max()
    blimp_offline /= blimp_offline.max()

    key_order = {
            "LookupTable":['size','input_min','input_max', 'isPeriodic'],
            "ResampledLookupTable":['size','blimp_online','blimp_offline'],
            "BlimpTable":['size','intervals','res']
            }
    tables = [
            ['BLIMP_TABLE_OFFLINE',dict(classname="BlimpTable", data=blimp_offline,
                size=len(blimp_offline),
                intervals=offline_intervals,
                res=offline_res)],
            ['BLIMP_TABLE_ONLINE',dict(classname="BlimpTable", data=blimp_online,
                size=len(blimp_online),
                intervals=online_intervals,
                res=online_res)],
            ['PITCH_TABLE',dict(classname="LookupTable", data=pitchtable,
                size=len(pitchtable),
                input_min=0,
                input_max=1,
                isPeriodic=False)],
            ['BL_SAW',dict(classname="ResampledLookupTable", data=blsaw,
                size=len(blsaw),
                blimp_online="lut_blimp_table_online",
                blimp_offline="lut_blimp_table_offline")],
            ['SIN',dict(classname="LookupTable",data=sintable,
                size=len(sintable),
                input_min = 0,
                input_max = 1,
                isPeriodic = True)],
            ]
    macrodefines = [
            ]

    # Generate macro defines
    macrodefine_code = ""
    for macroname,macroval in macrodefines:
        line = "#define {} {}".format(macroname, macroval)
        macrodefine_code += line + "\n"
    # Generate lookup table structures
    tabledata_def = ""
    tabledata_decl = ""
    tableobj_def = ""
    for name,struct in tables:
        tabledata_def += MakeTableStr(struct['data'],name)
        tabledata_decl += "extern const double {}[{}];\n".format(name,len(struct['data']))

        currargs = []
        classname = struct["classname"]
        for k in key_order[classname]:
            if k not in struct:
                continue
            if k=="data":
                continue
            if k=="classname":
                continue
            val = struct[k]
            if type(val)==bool:
                val = "true" if val else "false"
            currargs.append("{}".format(val))
        tableargs = "{}, ".format(name)+", ".join(currargs)
        tableobj_currdef = "const {} lut_{}({});\n".format(classname,name.lower(),tableargs);
        tableobj_def += tableobj_currdef

    if os.path.isfile("table_data.h"):
        if v:
            print "Backing up old table_data.h..."
        old_table_data = open('table_data.h','r').read()
        with open('table_data.h.bk','w') as fp:
            fp.write(old_table_data)

    if v:
        print "Writing new table_data.cpp..."
    with open('table_data.h','w') as fp:
        fp.write(tabledata_def)

    if os.path.isfile("tables.h"):
        if v:
            print "Backing up old tables.h..."
        old_code = open('tables.h','r').read()
        with open('tables.h.bk','w') as fp:
            fp.write(old_code)
    if v:
        print "Writing new tables.h..."
    new_code = rewriteAutomatedSection(old_code,'lut_defs', tabledata_decl+'\n'+tableobj_def)
    new_code = rewriteAutomatedSection(new_code,'macro_defs', macrodefine_code)
    with open('tables.h', 'w') as fp:
        fp.write(new_code)

if __name__=="__main__":
    import argparse as ap
    psr = ap.ArgumentParser()
    psr.add_argument("-v","--verbose",action="store_true")
    pargs = psr.parse_args()

    main(pargs)
