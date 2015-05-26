import sys
import os
import os.path
indir = sys.argv[1]
outdir = sys.argv[2]
tmpdir = sys.argv[3]

def getword(fname):
    fp = open(fname, 'r')
    line = fp.read()
    word = line.replace("\n", "")
    word = word.replace(" ", "")
    word = word.replace("\t", "")
    return word

def ocr(pdir, tmpdir, filename, prefix):
    str_layer=""
    os.system("./jk %s %s"%(os.path.join(pdir, filename), os.path.join(tmpdir, prefix)))
    for parent, dirnames, filenames in os.walk(tmpdir):
        for fname in filenames:
            postfix = fname.split('.')[-1];
            if postfix=="jpg" or postfix=="JPG" or postfix=="png" or postfix=="PNG":
                print fname,"tesseract"
                str_cn = ""
                str_en = ""
                prefix_ch = "%s_ch"%(os.path.splitext(fname)[0])
                prefix_en = "%s_en"%(os.path.splitext(fname)[0])
                outfile_ch=os.path.join(tmpdir, prefix_ch)
                os.system("tesseract %s %s -l chi_sim config.txt"%(os.path.join(parent, fname), outfile_ch))
                if os.path.exists("%s.txt"%outfile_ch):
                    str_ch = getword("%s.txt"%outfile_ch)
                    print "str_cn=%s, len=%d"%(str_cn, len(str_cn))

                outfile_en=os.path.join(tmpdir, prefix_en)
                os.system("tesseract %s %s -l eng config.txt"%(os.path.join(parent, fname), outfile_en))
                if os.path.exists("%s.txt"%outfile_en):
                    str_en = getword("%s.txt"%outfile_en)
                    print "str_en=%s, len=%d"%(str_en, len(str_en))

                if len(str_ch)>0 or len(str_en)>0:
                    str_layer = str_layer + "%s\n%s"%(str_ch, str_en)

    os.system("rm -f %s/*"%(tmpdir))
    if len(str_layer)>0:
        return (True, str_layer)

    return (False, "")
    


    

for parent, dirnames, filenames in os.walk(indir):
    for filename in filenames:
        postfix = filename.split('.')[-1];
        if postfix=="jpg" or postfix=="JPG" or postfix=="png" or postfix=="PNG":
            print filename
            print "lll"
            prefix = os.path.splitext(filename)[0]
            has_char,cs = ocr(parent, tmpdir, filename, prefix)
            if has_char == True:
                outfile = os.path.join(outdir, "%s.result"%prefix)
                fout = open(outfile, "w")
                fout.write(cs)
                fout.close()



