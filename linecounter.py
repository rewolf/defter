
import os
import sys

exts    = ['c', 'cpp','h','frag','vert','geom']
exclude = ['glew.c']
perfile = []

count   = 0
lines   = 0

if len(sys.argv) > 1:
    if sys.argv[1] in ['help','--help','-h']:
        print "Usage: python linecounter.py [extension1] [extension2] ... [extension3]"
        print "\textensionN = extension of files to process"
    else:
        exts = argv[1:]

for root, dirs, files in os.walk('.'):
    if '.git' not in root:
        for f in files:
            dot = f.rfind('.')
            if dot != -1:
                ext = f[dot+1:]
                if ext in exts and f not in exclude:
                    count += 1
                    fp = open(os.path.join(root,f),'r')
                    flines = len(fp.readlines())
                    fp.close()
                    lines += flines
                    perfile.append((f,flines))
                    
perfile.sort(key=lambda x:x[1], reverse=True)

print "\n%d Files, %d Lines" % (count, lines)
print "\nTop Files:"
for f, n in perfile[:5]:
    print f.ljust(20,' ') + `n`.rjust(5,' ')