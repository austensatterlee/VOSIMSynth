import numpy as np
from numpy import *
from scipy import signal as ss
import re
import os, sys

script_dir = os.path.realpath(os.path.dirname(__file__))
LUT_TABLEDATA_FILE = os.path.join(script_dir, "src/table_data.cpp")
LUT_TABLEHDR_FILE = os.path.join(script_dir, "include/vosimlib/lut_tables.h")
LUT_TABLESRC_FILE = os.path.join(script_dir, "src/lut_tables.cpp")

def RealCepstrum(n, signal):
    freq = fft.fft(signal)
    freq = log(abs(freq))
    realCepstrum = real(fft.ifft(freq))

    return realCepstrum

def MinimumPhase(n, realCepstrum):
    nd2 = n/2
    realTime = zeros(n)
    imagTime = zeros(n)
    realTime[0] = realCepstrum[0]
    realTime[1:nd2] = 2*realCepstrum[1:nd2]
    if ((n%2)==0):
        realTime[nd2] = realCepstrum[nd2]

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
    x = (a + arange(n)/float(n-1)*(b-a))
    buffer1 = sinc(x)

    buffer1 *= blackman(n)

    buffer2 = RealCepstrum(n, buffer1)
    buffer1 = MinimumPhase(n, buffer2)

    minBLEP = buffer1.cumsum()

    minBLEP/=minBLEP[n-1]

    return minBLEP

def DSF(w, fc, fm, nharmonics, npoints=None):
    """
    Digital summation formula.

    Computes the sum of (w^k sin(u + k*v)) from k=0 to `nharmonics`.
    u = 2*pi*fc*n/npoints for n=0 to npoints-1
    v = 2*pi*fm*n/npoints for n=0 to npoints-1

    """
    npoints = npoints or 2*nharmonics+1
    sample_pts = arange(npoints)*2.0*pi/npoints
    u = fc*sample_pts
    v = fm*sample_pts

    nh = nharmonics
    # num = (w*sin(v-u) + sin(u)) + w**(nh+1) * (w * sin(u + nh*v) - sin(u + (nh+1)*v))
    # den = (1 + w*w - 2*w*cos(v))
    cnum = exp(1j*u) * (1 - w**(nh+1)*exp(1j*v*(nh+1)))
    cden = (1 - w*exp(1j*v))

    limmask = where((cden==0) & (cnum==0))
    result = cnum / cden
    result[limmask] = 1.0
    return result

def RCCoefs(fc, fs):
    """
    fn - normalized frequency

    @returns bcoefs, acoefs
    """
    T = 1./fs
    RC = 1./(2*pi*fc)
    bcoefs = array([1., 1.])
    acoefs = array([1.+2*RC/T, 1.-2*RC/T])
    bcoefs /= acoefs[0]
    acoefs /= acoefs[0]
    return bcoefs, acoefs

def GenerateBlit(pts, nharmonics=None):
    """
    Generate a band-limited impulse train using the FFT

    """
    nharmonics = nharmonics or (pts/2 if pts%2!=0 else pts/2-1)
    fftpts = pts/2+1
    blit_spectrum = zeros(fftpts)
    blit_spectrum[:nharmonics+1] = 1.0
    blit = fft.fftshift(fft.irfft(blit_spectrum))
    return blit

def bl_prefilter():
    prefilter_lhalf=[0.0028, -0.0119, 0.0322, -0.0709, 0.1375, -0.2544, 0.4385, -0.6334, 1.7224]
    prefilter=make_symmetric_l(prefilter_lhalf)
    return prefilter

def GenerateBLSaw(nharmonics, npoints=None, w0=1):
    """ Generate band limited sawtooth wave """
    npoints = npoints or nharmonics*2+1
    nharmonics = min(nharmonics, npoints/2-1 if npoints%2 else npoints/2)

    harmonics = [w0*x for x in xrange(1, nharmonics+1)]

    gains = [1./x for x in harmonics]

    sample_pts = arange(npoints)*2.0*pi/npoints
    blsaw = sum([g*sin(h*sample_pts) for g, h in zip(gains, harmonics)], axis=0)
    blsaw = -fft.fftshift(blsaw)
    return blsaw/blsaw.max()

def GenerateBLSquare(nharmonics, npoints=None, w0=1):
    npoints = npoints or nharmonics*2+1
    nharmonics = min(nharmonics, npoints/2-1 if npoints%2 else npoints/2)

    harmonics = [w0*x for x in xrange(1, nharmonics+1) if x%2]

    gains = [1./x for x in harmonics]

    sample_pts = arange(npoints)*2.0*pi/npoints;
    blsquare = sum([g*sin(h*sample_pts) for g, h in zip(gains, harmonics)], axis=0)
    return blsquare/blsquare.max()

def GenerateBLTriangle(nharmonics, npoints=None, w0=1):
    npoints = npoints or nharmonics*2+1
    nharmonics = min(nharmonics, npoints/2-1 if npoints%2 else npoints/2)

    harmonics = [w0*x for x in xrange(1, nharmonics+1) if x%2]

    gains = [1./x**2 if not i%2 else -1./x**2 for i, x in enumerate(harmonics)]

    sample_pts = arange(npoints)*2.0*pi/npoints
    bltri = sum([g*sin(h*sample_pts) for g, h in zip(gains, harmonics)], axis=0)
    return bltri/bltri.max()

def GenerateBlimp(intervals=10, res=2048, fs=48000, fc=23000, beta=7.20, apgain=0.89, apbeta=0.7, ret_half=True):
    """
    Generate a bandlimited dirac delta impulse.

    intervals - number of samples (sampled at fs) to span
    res - number of points per sample; e.g. oversampling amount
    fs - sampling frequency
    fc - cutoff
    beta - first window param
    apgain - apodizing window gain
    """
    intervals=2*(int(intervals)/2)+1 # make intervals odd
    res = 2*(int(res)/2)+1 # make oversample factor odd
    pts = res*intervals # impulse length in points
    ind = intervals*2*(arange(pts) - (pts-1)/2.)/(pts-1.) # pts points spread across [-intervals/2, intervals/2)
    x = fc*1./fs*ind # pts points spread across fc/fs*[-intervals/2, intervals/2)
    h = sinc(x)

    w = ss.kaiser(pts, beta) # window
    apw = 1-apgain*ss.kaiser(pts, apbeta) # apodization window
    window = w*apw
    blimp = window*h
    blimp = ss.fftconvolve(blimp, blimp, 'same')

    # Scale
    blimp = scale_fir(blimp)

    if ret_half:
        return blimp[len(blimp)/2:]
    else:
        return blimp

def GenerateBlimp2(intervals=10, res=2048, fs=48e3, fc=20e3, beta=7.20, apgain=0.89, apbeta=0.7, ret_half=True, width=None):
    intervals=2*(int(intervals)/2)+1 # make intervals odd
    res = 2*(int(res)/2)+1 # make oversample factor odd
    pts = res*intervals # impulse length in points
    blimp_fir = ss.firwin(pts, 2*fc*1.0/fs*1.0/res, width=width, window=('kaiser', beta))

    apwin = 1-apgain*ss.kaiser(pts, apbeta) # apodization window
    blimp_fir *= apwin
    blimp_fir = ss.fftconvolve(blimp_fir, blimp_fir, 'same')

    # Scale
    blimp_fir = scale_fir(blimp_fir)

    if ret_half:
        half = blimp_fir[len(blimp_fir)/2:]
        return half

    return blimp_fir

def GenerateSine(n):
    k = arange(n)
    T = 1./n
    c = sin(k*T*2*pi)
    return c

def GeneratePitchTable(notestart, notefinish, res):
    """ Generate a table that maps midi notes to frequency

    notestart - lowest midi note
    notefinish - highest midi note
    res - number of points between midi notes

    returns (midi notes, frequencies)
    """
    npoints = (notefinish-notestart)*res+1
    N = linspace(notestart, notefinish, npoints)
    return N, 440*2**((N-69.)/12.)

"""Tools"""
def RewriteAutomatedSection(full_text, tag, replacement_text):
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
    replacement_text=re.sub("\n\\s*", "\n"+match.groupdict()['spacing'], replacement_text)
    result = full_text[:match.start("block")] + replacement_text + full_text[match.end("block"):]
    return result

def MakeTableStr(table, name, ctype="double"):
    rows = int(sqrt(table.size))
    cols = int(ceil(sqrt(table.size)))
    tablestr = "{} {}[{}] = {{\n".format(ctype, name, table.size)
    cellstr = "{:.18f}"
    ind = 0
    for i in xrange(rows):
        if i!=0:
            tablestr+='\n'
        for j in xrange(cols):
            tablestr+=cellstr.format(table[ind])
            tablestr+=', '
            ind+=1
            if ind>=table.size:
                break
        if ind>table.size:
            break
    tablestr+="\n};\n"
    return tablestr

def make_symmetric_r(right_half):
    left_half = list(reversed(right_half))
    return hstack([left_half[:-1], right_half])

def make_symmetric_l(left_half):
    right_half = list(reversed(left_half))
    return hstack([left_half[:-1], right_half])

def clipdb(s, cutoff):
    clipped = s[:]
    spos = abs(s)
    mspos = max(spos)
    if not mspos:
        return s
    if cutoff >= 0:
        return s
    thresh = mspos*10**(cutoff/20.)
    clipped[spos<thresh] = thresh
    return clipped

def magspec(signal, pts=None, fs=None, xlog=False, ylog=False, ax=None, **kwargs):
    from matplotlib import pyplot as plt
    pts = pts or len(signal)
    k = arange(1, pts/2)
    fs = fs or 1.
    xlogscale, ylogscale = (xlog, ylog)
    xlogscale = kwargs.pop('xlog', xlogscale)
    ylogscale = kwargs.pop('ylog', ylogscale)
    signalfft = fft.rfft(signal, n=pts)
    freqs = arange(len(signalfft))*1./len(signalfft)*fs/2.0
    if ylogscale:
        signalfft = clipdb(signalfft, -180)
        mag = 20*log10(abs(signalfft))
        ylabel = "dB"
    else:
        mag = abs(signalfft)**2
        ylabel = "Amp"
    ax = ax or plt.gca()
    if xlogscale:
        ax.semilogx( freqs, mag, basex=10, **kwargs)
    else:
        ax.plot( freqs, mag, **kwargs)
    ax.set_xlabel("Hz")
    ax.set_ylabel(ylabel)
    ax.grid(True, which='both')

def maggain(signal1, signal2, pts=None, fs=None, xlog=False, ylog=False, **kwargs):
    from matplotlib import pyplot as plt
    fftlength = pts or max(len(signal1), len(signal2))
    fs = fs or 1.
    xlogscale, ylogscale = (xlog, ylog)
    xlogscale = kwargs.pop('xlog', xlogscale)
    ylogscale = kwargs.pop('ylog', ylogscale)
    signal1fft = fft.rfft(signal1, n=fftlength)
    signal2fft = fft.rfft(signal2, n=fftlength)
    k = arange(len(signal1fft))
    freqs = k*1./fftlength*fs
    if ylogscale:
        signal1fft = clipdb(signal1fft, -180)
        signal2fft = clipdb(signal2fft, -180)
        mag1 = 20*log10(abs(signal1fft))
        mag2 = 20*log10(abs(signal2fft))
        gain = mag2-mag1
        ylabel = "dB"
    else:
        mag1 = abs(signal1fft)**2
        mag2 = abs(signal2fft)**2
        gain = [m2/m1 if m1 else 0.0 for m2, m1 in zip(mag2, mag1)]
        ylabel = "Amp"
    f = plt.gcf()
    ax = plt.gca()
    if xlogscale:
        ax.semilogx( freqs, gain, basex=10, **kwargs)
    else:
        ax.plot( freqs, gain, **kwargs )
    ax.set_xlabel("Hz")
    ax.set_ylabel(ylabel)
    ax.grid(True, which='both')
    f.show()

def unitplot(*sigs, **kwargs):
    """
    plots multiple signals on the same timescale

    """
    from matplotlib import pyplot as plt
    f = plt.gcf()
    ax = plt.gca()
    pltsigs = []
    for sig in sigs:
        pltsigs.append( linspace(0, 1, len(sig)) )
        pltsigs.append( sig )
    ax.plot(*pltsigs, **kwargs)
    ax.grid(True, which="both")
    f.show()

def lerp(a, b, frac):
    return (b-a)*frac + a

def invlerp(a, b, pt):
    return (pt-a)*1.0/(b-a)

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
                next_filt_phase = clip(int(filt_phase)+1, 0, len(filt)-1)
                frac_filt_phase = filt_phase - int(filt_phase)
                filt_sample = lerp(filt[int(filt_phase)], filt[next_filt_phase], frac_filt_phase)
            else:
                filt_sample = filt[int(filt_phase)]
            sig_index = index - i
            if isperiodic:
                sig_index = mod(sig_index, siglen)
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
                next_filt_phase = clip(int(filt_phase)+1, 0, len(filt)-1)
                frac_filt_phase = filt_phase - int(filt_phase)
                filt_sample = lerp(filt[int(filt_phase)], filt[next_filt_phase], frac_filt_phase)
            else:
                filt_sample = filt[int(filt_phase)]
            sig_index = index + 1 + i
            if isperiodic:
                sig_index = mod(sig_index, siglen)
                newsig[new_index] += filt_sample * signal[sig_index]
            elif sig_index >= 0 and sig_index < siglen:
                newsig[new_index] += filt_sample * signal[sig_index]
            filt_phase += filt_step
            filt_sum += filt_sample
            i+=1
        num_taps_used += i-1
        newsig[new_index] = newsig[new_index]/filt_sum
        if num_taps_used > max_taps_used:
            max_taps_used = num_taps_used
        phase += phase_step
        new_index+=1
        if phase >= 1:
            phase -= 1
            currperiod+=1
    return newsig

def sinclowpass(signal, fc, winlen=10):
    newsignal = zeros(len(signal))
    M = arange(len(signal))
    # for m in M:
        # for n in xrange(N):
            # newsignal[m] += signal[n]*sinc(fc*(m-n))
        # newsignal[m]*=2*fc
    fc = fc
    sincfir = sinc(fc*arange(-winlen+1, winlen+1))
    apwin = genApodWindow(len(sincfir), 9., 0.9, 0.7)
    sincfir *= apwin
    sincfir /= sincfir.max()
    for m in M:
        for n in xrange(m-winlen+1, m+winlen+1):
            k = clip(n, 0, len(signal)-1)
            newsignal[m] += signal[k]*sincfir[m-n+winlen]
    return newsignal*fc

def genApodWindow(pts, beta, apgain, apbeta):
    w = ss.kaiser(pts, beta) # window
    apw = 1-apgain*ss.kaiser(pts, apbeta) # apodization window
    W = w*apw
    return W

def shift(signal, dPhase):
    return roll(signal, int(len(signal)*dPhase))

def power(signal):
    return sqrt(sum(signal**2)/len(signal))

def norm_power(signal):
    input_power = power(signal)
    output_power = sqrt(2.)/2.
    signal_norm = signal/input_power*output_power
    return signal_norm

def scale_fir(fir):
    pts = len(fir)

    # fresp = fft.rfft(fir)
    # dcresp = abs(fresp)[0]
    # return fir/dcresp

    alpha = 0.5 * (pts - 1)
    m = np.arange(0, pts) - alpha
    c = np.cos(np.pi * m * 0.0)
    s = np.sum(fir * c)
    return fir/s

def transition_band(mags, freqs=None, stopband_db=-90):
    """ Calculate the start and stop of the transition band from a frequency response """
    freqs = freqs if freqs is not None else np.arange(len(mags))
    mags = abs(mags)
    passband_mag = 10**(-3/20.)
    stopband_mag = 10**(stopband_db/20.)
    passband_index = np.where((mags-passband_mag)<=0)[0][0]
    stopband_index = np.where((mags-stopband_mag)<=0)[0][0] if min(mags)<stopband_mag else len(mags)-1
    return np.array([freqs[passband_index], freqs[stopband_index]])

def passband_ripple(mags):
    """ Calculate the passband ripple (maximum deviation from unity) from a frequency response """
    mags = abs(mags)
    passband_mag = 10**(-3/20.)
    passband_index = np.where((mags-passband_mag)<=0)[0][0]
    passband = mags[:passband_index+1]
    ripple = max(passband-passband.mean())
    return np.array(ripple)

def main(pargs):
    v = pargs.verbose
    clean = pargs.clean

    """Sin table"""
    if v:
        sys.stdout.write("Generating sine table...")
    SINE_RES = 1024
    sintable = GenerateSine(SINE_RES)
    if v:
        print " samples:", len(sintable)

    """Pitch table"""
    if v:
        print "Generating pitch table..."
    PITCH_RES = 10
    MIN_PITCH = -128
    MAX_PITCH = 256
    pitches, pitchtable = GeneratePitchTable(MIN_PITCH, MAX_PITCH, PITCH_RES)

    """Prefilter for bandlimited waveforms"""
    prefilter = bl_prefilter()

    """Bandlimited saw wave"""
    if v:
        sys.stdout.write("Generating band-limited saw wavetable...")
    BLSAW_HARMONICS = 4096
    blsaw = GenerateBLSaw(BLSAW_HARMONICS)
    blsaw = ss.fftconvolve( blsaw, prefilter, 'same' ) # apply gain to high frequencies
    blsaw /= blsaw.max()
    if v:
        print " samples:", len(blsaw)

    """Bandlimited square wave"""
    if v:
        sys.stdout.write("Generating band-limited square wavetable...")
    BLSQUARE_HARMONICS = 4096
    blsquare = GenerateBLSquare(BLSQUARE_HARMONICS)
    blsquare = ss.fftconvolve( blsquare, prefilter, 'same' )
    blsquare /= blsquare.max()
    if v:
        print " samples:", len(blsquare)

    """Bandlimited triangle wave"""
    if v:
        sys.stdout.write("Generating band-limited triangle wavetable...")
    BLTRI_HARMONICS = 4096
    bltri = GenerateBLTriangle(BLTRI_HARMONICS)
    bltri = ss.fftconvolve( bltri, prefilter, 'same' )
    bltri /= bltri.max()
    if v:
        print " samples:", len(bltri)

    """Offline and online BLIMP for resampling"""
    OFFLINE_BLIMP_INTERVALS = 513
    OFFLINE_BLIMP_RES = 2048

    ONLINE_BLIMP_INTERVALS = 19
    ONLINE_BLIMP_RES = 2048

    if v:
        sys.stdout.write("Generating 'online' band-limited impulse table...")
    blimp_online = GenerateBlimp(ONLINE_BLIMP_INTERVALS, ONLINE_BLIMP_RES, fc=23e3, fs=48e3)
    if v:
        print " samples:", len(blimp_online)

    if v:
        sys.stdout.write("Generating 'offline' band-limited impulse table...")
    blimp_offline = GenerateBlimp(OFFLINE_BLIMP_INTERVALS, OFFLINE_BLIMP_RES, fc=24e3, fs=48e3)
    if v:
        print " samples:", len(blimp_offline)

    if v:
        print "Generating C++ code..."

    """Generate C++ files"""
    NAMESPACE = "syn"

    key_order = {
            "NormalTable":['size'],
            "AffineTable":['size', 'input_min', 'input_max'],
            "ResampledTable":['size', 'blimp_online', 'blimp_offline'],
            "BlimpTable":['size', 'intervals', 'res']
            }
    tables = [
            ['BLIMP_TABLE_OFFLINE', dict(classname="BlimpTable", data=blimp_offline,
                size=len(blimp_offline),
                intervals=OFFLINE_BLIMP_INTERVALS,
                res=OFFLINE_BLIMP_RES)],
            ['BLIMP_TABLE_ONLINE', dict(classname="BlimpTable", data=blimp_online,
                size=len(blimp_online),
                intervals=ONLINE_BLIMP_INTERVALS,
                res=ONLINE_BLIMP_RES)],
            ['PITCH_TABLE', dict(classname="AffineTable", data=pitchtable,
                size=len(pitchtable),
                input_min = min(pitches),
                input_max = max(pitches))],
            ['BL_SAW_TABLE', dict(classname="ResampledTable", data=blsaw,
                size=len(blsaw),
                blimp_online="lut_blimp_table_online()",
                blimp_offline="lut_blimp_table_offline()")],
            ['BL_SQUARE_TABLE', dict(classname="ResampledTable", data=blsquare,
                size=len(blsquare),
                blimp_online="lut_blimp_table_online()",
                blimp_offline="lut_blimp_table_offline()")],
            ['BL_TRI_TABLE', dict(classname="ResampledTable", data=bltri,
                size=len(bltri),
                blimp_online="lut_blimp_table_online()",
                blimp_offline="lut_blimp_table_offline()")],
            ['SIN_TABLE', dict(classname="NormalTable", data=sintable,
                size=len(sintable))]
            ]
    macrodefines = [
            ]

    # Generate macro defines
    macrodefine_code = ""
    for macroname, macroval in macrodefines:
        line = "#define {} {}".format(macroname, macroval)
        macrodefine_code += line + "\n"

    # Generate lookup table structures
    tabledata_def = ""
    tabledata_decl = ""
    tableobjfuncs_decl = ""
    tableobjfuncs_def = ""
    for name, struct in tables:
        tabledata_def += MakeTableStr(struct['data'], name)
        tabledata_decl += "extern double {}[];\n".format(name)

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
        tableobjfunc_currdecl = """{0:}& VOSIMLIB_API lut_{1:}();\n""".format(classname, name.lower())
        tableobjfunc_currdef = """{0:}& lut_{1:}() {{ static {0:} table({2:}); return table; }}\n""".format(classname, name.lower(), tableargs)
        tableobjfuncs_decl += tableobjfunc_currdecl
        tableobjfuncs_def += tableobjfunc_currdef

    # Backup old table data file
    if os.path.isfile(LUT_TABLEDATA_FILE):
        if v:
            print "Backing up old {}...".format(LUT_TABLEDATA_FILE)
        old_table_data = open(LUT_TABLEDATA_FILE, 'r').read()
        with open(LUT_TABLEDATA_FILE+'.bk', 'w') as fp:
            fp.write(old_table_data)

    # Write new table data file
    if v:
        print "Writing new {}...".format(os.path.realpath(LUT_TABLEDATA_FILE))
    with open(LUT_TABLEDATA_FILE, 'w') as fp:
        # Add code to namespace
        tabledata_def = """namespace {} {{
    {}
}}""".format(NAMESPACE, tabledata_def)
        fp.write(tabledata_def)

    # Back up old table header file
    old_code = ""
    if os.path.isfile(LUT_TABLEHDR_FILE):
        if v:
            print "Backing up old {}...".format(LUT_TABLEHDR_FILE)
        old_code = open('{}'.format(LUT_TABLEHDR_FILE), 'r').read()
        with open('{}.bk'.format(LUT_TABLEHDR_FILE), 'w') as fp:
            fp.write(old_code)

    if v:
        print "Writing new {}...".format(os.path.realpath(LUT_TABLEHDR_FILE))
    if clean:
        old_code = ""
    # Add tags if the file is empty
    if not old_code: 
        old_code=\
"""#pragma once

#include "vosimlib/common.h"

/*::macro_defs::*/
/*::/macro_defs::*/

namespace {}{{
    /*::lut_decl::*/
    /*::/lut_decl::*/
}}
""".format(NAMESPACE)

    class_fwd_decls = "\n".join(["class {};".format(cls) for cls in key_order.keys()])
    new_code = RewriteAutomatedSection(old_code, 'lut_decl', class_fwd_decls + "\n\n" + tableobjfuncs_decl)
    new_code = RewriteAutomatedSection(new_code, 'macro_defs', macrodefine_code)
    with open(LUT_TABLEHDR_FILE, 'w') as fp:
        fp.write(new_code)

    # Back up old table src file
    old_code = ""
    if os.path.isfile(LUT_TABLESRC_FILE):
        if v:
            print "Backing up old {}...".format(LUT_TABLESRC_FILE)
        old_code = open('{}'.format(LUT_TABLESRC_FILE), 'r').read()
        with open('{}.bk'.format(LUT_TABLESRC_FILE), 'w') as fp:
            fp.write(old_code)

    if v:
        print "Writing new {}...".format(os.path.realpath(LUT_TABLESRC_FILE))
    if clean:
        old_code=""
    # Add tags if the file is empty
    if not old_code:
        old_code=\
"""#include "vosimlib/tables.h"
#include "vosimlib/lut_tables.h"

namespace {} {{
    /*::table_decl::*/
    /*::/table_decl::*/

    /*::lut_defs::*/
    /*::/lut_defs::*/
}}
""".format(NAMESPACE)

    new_code = RewriteAutomatedSection(old_code, 'lut_defs', tableobjfuncs_def)
    new_code = RewriteAutomatedSection(new_code, 'table_decl', tabledata_decl)
    with open(LUT_TABLESRC_FILE, 'w') as fp:
        fp.write(new_code)

if __name__=="__main__":
    import argparse as ap
    psr = ap.ArgumentParser()
    psr.epilog = "Output files: {}".format(", ".join([LUT_TABLESRC_FILE, LUT_TABLEHDR_FILE, LUT_TABLEDATA_FILE]))
    psr.add_argument("-v", "--verbose", action="store_true")
    psr.add_argument("-c", "--clean", action="store_true")
    pargs = psr.parse_args()

    main(pargs)
