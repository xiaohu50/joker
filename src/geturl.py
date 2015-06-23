import sys
index=int(sys.argv[1])
total=int(sys.argv[2])
fin=open(sys.argv[3],'r')
fout=open(sys.argv[4], 'w')
line=fin.readline()
i=0
while line:
    if i%total==index:
        line = line.rstrip("\n")
        info = line.split("\t")
        uid = info[0]
        url = info[3]
        newurl = "%s%s.jpg"%(url[0:-1], uid)
        fout.write(newurl+"\n")
    line=fin.readline()
    i += 1
fin.close()
fout.close()



