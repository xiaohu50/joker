import sys
import os
import os.path
indir = sys.argv[1]
outdir = sys.argv[2]
for parent, dirnames, filenames in os.walk(indir):
    for filename in filenames:
        postfix = filename.split('.')[-1];
        if postfix=="jpg" or postfix=="JPG" or postfix=="png" or postfix=="PNG":
            print filename
            prefix = os.path.splitext(filename)[0]
            os.system("./jk %s %s"%(os.path.join(parent, filename), os.path.join(outdir, prefix)))

for parent, dirnames, filenames in os.walk(outdir):
    for filename in filenames:
        postfix = filename.split('.')[-1];
        if postfix=="jpg" or postfix=="JPG" or postfix=="png" or postfix=="PNG":
            print filename
            prefix_ch = "%s_ch"%(os.path.splitext(filename)[0])
            prefix_en = "%s_en"%(os.path.splitext(filename)[0])
            outfile_ch=os.path.join(outdir, prefix_ch)
            os.system("tesseract %s %s -l chi_sim config.txt"%(os.path.join(parent, filename), outfile_ch))
            if os.path.exists("%s.txt"%outfile_ch):
                size = os.path.getsize("%s.txt"%outfile_ch)
                if size==0:
                    os.remove("%s.txt"%outfile_ch)

            outfile_en=os.path.join(outdir, prefix_en)
            os.system("tesseract %s %s -l eng config.txt"%(os.path.join(parent, filename), outfile_en))
            if os.path.exists("%s.txt"%outfile_en):
                size = os.path.getsize("%s.txt"%outfile_en)
                if size==0:
                    os.remove("%s.txt"%outfile_en)
