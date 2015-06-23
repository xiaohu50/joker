# -*- coding:utf-8 -*-
import os
import os.path
import sys
reload(sys)
sys.setdefaultencoding("utf-8")

url_old = sys.argv[1]
suspiciousDir = sys.argv[2]
spamDir = sys.argv[3]
suspicious_file = sys.argv[4]
spam_file = sys.argv[5]

spam_uids=dict()
suspicious_uids=dict()
for parent, dirnames, filenames in os.walk(suspiciousDir):
    for filename in filenames:
        postfix = filename.split('.')[-1];
        if postfix=="jpg" or postfix=="JPG" or postfix=="png" or postfix=="PNG":
            uid = os.path.splitext(filename)[0]
            suspicious_uids[uid]=1

for parent, dirnames, filenames in os.walk(spamDir):
    for filename in filenames:
        postfix = filename.split('.')[-1];
        if postfix=="jpg" or postfix=="JPG" or postfix=="png" or postfix=="PNG":
            uid = os.path.splitext(filename)[0]
            spam_uids[uid]=1

furl = open(url_old, "r")
fsuspicious = open(suspicious_file, "w")
fspam = open(spam_file, "w")
line = furl.readline()
while line:
    info = line.split("\t")
    uid = info[0]
    if uid in suspicious_uids:
        fsuspicious.write(line)
    if uid in spam_uids:
        fspam.write(line)

    line = furl.readline()
fsuspicious.close()
fspam.close()

