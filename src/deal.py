# -*- coding:utf-8 -*-
import os
import os.path
import sys
reload(sys)
sys.setdefaultencoding("utf-8")

indir = sys.argv[1]
tmpdir = sys.argv[2]
spam_dict_file = sys.argv[3]
bad_uid_file = sys.argv[4]
suspiciousDir = sys.argv[5]
spamDir = sys.argv[6]

if len(sys.argv)>7:
    outdir = sys.argv[3]
    

def loadSpamDict(fname):
    spam_dict = dict()
    spam_dict["level1"]=dict()
    spam_dict["level2"]=dict()
    fp = open(fname, "r")
    line = fp.readline()
    while line:
        line = line.decode("utf-8")
        line = line.rstrip("\n")
        if len(line)>0:
            info = line.split("\t")
            for i in range(2, len(info)):
                spam_dict["level%s"%(info[0])][info[i]] = info[1]
        line = fp.readline()
    fp.close()
    return spam_dict

def getSpamWord(spam_dict, line):
    spam_word = ""
    number_count = 0
    for c in line:
        if c in spam_dict["level1"] or c in spam_dict["level2"]:
            spam_word += c
        if c.isdigit():
            number_count += 1
    if number_count>7:
        spam_word += '1'

    return spam_word

def isSpam(spam_dict, word):
    level1 = dict()
    level2 = dict()
    for c in word:
        if c=="1":
            level2['number']=1
        else:
            if c in spam_dict['level1']:
                level1[spam_dict['level1'][c]]=1
            elif c in spam_dict['level2']:
                level2[spam_dict['level2'][c]]=1
    if len(level1)>=2 or len(level2)>=3:
        return "spam"
    elif len(level1)>=1 or len(level2)>=1:
        return "suspicious"
    else:
        return "nospam"



def getword(fname):
    fp = open(fname, 'r')
    line = fp.read()
    line = line.decode("utf-8")
    word = line.replace("\n", "")
    word = word.replace(" ", "")
    word = word.replace("\t", "")
    return word

def ocr(pdir, tmpdir, filename, prefix):
    str_layer=""
    print os.path.join(pdir, filename), os.path.join(tmpdir, prefix)
    os.system("./jk %s %s"%(os.path.join(pdir, filename), os.path.join(tmpdir, prefix)))
    os.system("cp %s %s"%(os.path.join(pdir, filename), tmpdir))
    for parent, dirnames, filenames in os.walk(tmpdir):
        for fname in filenames:
            postfix = fname.split('.')[-1];
            if postfix=="jpg" or postfix=="JPG" or postfix=="png" or postfix=="PNG":
                print fname,"tesseract"
                str_ch = ""
                str_en = ""
                prefix_ch = "%s_ch"%(os.path.splitext(fname)[0])
                prefix_en = "%s_en"%(os.path.splitext(fname)[0])
                outfile_ch=os.path.join(tmpdir, prefix_ch)
                os.system("/usr/local/bin/tesseract %s %s -l chi_sim config.txt"%(os.path.join(parent, fname), outfile_ch))
                if os.path.exists("%s.txt"%outfile_ch):
                    str_ch = getword("%s.txt"%outfile_ch)
                    print "str_ch=%s, len=%d"%(str_ch, len(str_ch))

                outfile_en=os.path.join(tmpdir, prefix_en)
                os.system("/usr/local/bin/tesseract %s %s -l eng config.txt"%(os.path.join(parent, fname), outfile_en))
                if os.path.exists("%s.txt"%outfile_en):
                    str_en = getword("%s.txt"%outfile_en)
                    print "str_en=%s, len=%d"%(str_en, len(str_en))

                if len(str_ch)>0 or len(str_en)>0:
                    str_layer = str_layer + "%s%s\n"%(str_ch, str_en)

    os.system("rm -f %s/*"%(tmpdir))
    if len(str_layer)>0:
        return (True, str_layer)

    return (False, "")
    


    
spam_dict = loadSpamDict(spam_dict_file)
fuid=open(bad_uid_file, "w")
count=0
for parent, dirnames, filenames in os.walk(indir):
    for filename in filenames:
        postfix = filename.split('.')[-1];
        if postfix=="jpg" or postfix=="JPG" or postfix=="png" or postfix=="PNG":
            print "count=%d"%count
            count += 1
            print filename
            prefix = os.path.splitext(filename)[0]
            has_char,cs = ocr(parent, tmpdir, filename, prefix)
            if has_char == True:
                if 'outdir' in dir():
                    outfile = os.path.join(outdir, "%s.txt"%prefix)
                    fout = open(outfile, "w")
                    fout.write(cs)
                    fout.close()

                cs = cs.decode("utf-8")
                lines = cs.split("\n")
                spam_word=""
                for line in lines:
                    spam_word += getSpamWord(spam_dict, line)

                result = isSpam(spam_dict, spam_word)
                if result == "spam":
                    fuid.write("%s\t%s\n"%(prefix, spam_word.encode("utf-8")))
                    os.system("cp %s %s"%(os.path.join(parent, filename), spamDir))
                elif result == "suspicious":
                    os.system("cp %s %s"%(os.path.join(parent, filename), suspiciousDir))

fuid.close()

