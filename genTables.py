from numpy import *
import re

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

def PitchTable(notestart=0,notefinish=128):
    n = arange(notestart,notefinish)
    return 440*2**((n-69.)/12.)

def rewriteAutomatedSection(full_text,replacement_text):
    block_pattern = re.compile("/\*::(?P<tag>.*?)::\*/ *?\n(?P<block>.*)\n/\*::/(?P=tag)::\*/",re.DOTALL);
    match = block_pattern.search(full_text)
    if not match:
        return full_text
    return re.sub(re.escape(match.groupdict()["block"]), replacement_text, full_text)

def main():
    ss_points = 65536
    sinsquaredtable = GenerateSinSquared(ss_points)
    sk_points = 50
    sk_fc = 0.49
    sinckernel = SincKernel(sk_points,sk_fc)
    pitchtable = PitchTable(-128,128)

    key_order = ['size','input_min','input_max']
    tables = {
            'VOSIM_PULSE_COS':dict(data=sinsquaredtable,size=ss_points),
            'PITCH_TABLE':dict(data=pitchtable,size=len(pitchtable),input_min=-1,input_max=1)
            }

    tabledata_def = ""
    tabledata_decl = ""
    tableobj_def = ""
    # tableinfostr = '\n'.join(["#define {} {}".format(k,v) for k,v in tableinfo.items()])
    # tableinfostr += '\n'
    for name,struct in tables.items():
        tabledata_def += MakeTableStr(tables[name]['data'],name)
        tabledata_decl += "extern const double {}[{}];\n".format(name,len(tables[name]['data']))
        tableargs = "{}, ".format(name)+", ".join(["{}".format(struct[k]) for k in key_order if k!='data' and k in struct])
        tableobj_def += "const LookupTable lut_{}({});\n".format(name.lower(),tableargs);

    with open('table_data.cpp','w') as fp:
        fp.write("#include \"tables.h\"\n")
        fp.write("namespace syn {\n")
        fp.write(tabledata_def)
        fp.write("\n}\n")
    header_text = open('tables.h','r').read()
    with open('tables.h', 'w') as fp:
        fp.write(rewriteAutomatedSection(header_text, tabledata_decl+'\n'+tableobj_def))

if __name__=="__main__":
    main()
