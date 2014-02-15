import sys
import re
import operator

"""
arguments 1: output file name -> created as as sys.argv[1] + ".json"

these 4 files are expected to exist:
1 - full.txt - the phonetic data; format: word::syllable-syllable-syllable
2 - map.txt - syllable to stroke mapping data; format:  fragment::XPADDEDSTROKE  plus there are special breaks to designate
				which syllable fragment is being mapped -- note, keys not in the result are represented by Xs, 
3 - patch.txt - words that have custom definitions and should not be generated based on pronunciation; format: "stroke": "word",
				you could load a whole json file, minus the brackets, as a custom word list (in fact, you probably should ...)
4 - en.txt - word frequency data; format: word ### <- larger # = more frequent
"""

print "starting, this will take a while ..."

fileI = open("full.txt", "r")
filem = open("map.txt", "r")

unify = re.compile("['/^]")
fixu = re.compile("\\^y")

ast = re.compile("\\*")

sh = re.compile("sh")
ch = re.compile("ch")
th = re.compile("th")
andsub = re.compile("\\&")

qsub = re.compile("Q")
zsub = re.compile("Z")
vsub = re.compile("V")
wsub = re.compile("W")

ly = re.compile("LY")
al = re.compile("AL")
ness = re.compile("NESSES")
nes = re.compile("NESS")
uble1 = re.compile("-uh-buhl")
uble2 = re.compile("uh-buhl")
able = re.compile("ABLE")
ment = re.compile("MENT")
ments = re.compile("MENTS")
ment2 = re.compile("-ment")
exm = re.compile("-ek-s([aeiouy])")
exs = re.compile("^ek-s([aeiouy])")

er = re.compile("[^ey]eh?r")

idict = {}
mdict = {}
edict = {}
fdict = {}

words = {}
replace = {}

"""
this function splits a syllable into three parts,
it relies on the mapping file to determine what the vowels are
"""

def splitsyb(syb):
	begin = ""
	mid = ""
	end = ""
	cnt = 1
	while cnt <= len(syb):
		if syb[0:cnt] in idict:
			begin = syb[0:cnt]
		cnt = cnt+1
	cnt = len(begin)+1
	while cnt <= len(syb):
		if syb[len(begin):cnt] in mdict:
			mid = syb[len(begin):cnt]
		cnt = cnt+1
	cnt = len(begin) + len(mid) + 1
	while cnt <= len(syb):
		if syb[len(begin) + len(mid):cnt] in edict:
			end = syb[len(begin) + len(mid):cnt]
		cnt = cnt+1
		
		
	if mid == "" and ((begin + end) in edict):
		end = begin+end
		begin = ""
	if begin+mid+end == syb:
		if mid == "" and end != "":
			mid = "u"
			# mid = mid
		return [begin, mid, end]
	else:
		# print "failed " + syb
		return None

		
"""
read in the phonetic to stroke mapping file
"""
mode = {'FULL': fdict, 'INITIAL': idict, 'MID': mdict, 'END': edict}
cdict = False

mline = filem.readline()
while mline:
	mline = mline.strip()
	if mline in mode:
		cdict = mode[mline]
	else:
		# print mline
		mline = mline.split("::")
		cdict[mline[0]] = mline[1]
		temp = sh.sub("Q", mline[0])
		temp = ch.sub("Z", temp)
		temp = th.sub("V", temp)
		cdict[temp] = mline[1]
	mline = filem.readline()


line = fileI.readline()
mcount = 0
while line:
	mcount = mcount + 1
	line = line.strip()
	if line != "": 
		"""
		first we fiddle with the phontic pronciation
		here some syllables are compressed together to shorted results
		what you do here will depend on your source of phonetic data
		"""
		lines = line.split("::")
		text = fixu.sub("u", lines[1])
		
		text = unify.sub("", text)
		
		text = sh.sub("Q", text)
		text = ch.sub("Z", text)
		text = th.sub("V", text)
		
		text = ly.sub("lee", text)
		text = al.sub("uhl", text)
		text = able.sub("ubl", text)
		text = ments.sub("muhnt-S", text)
		text = ment.sub("muhnt", text)
		text = ness.sub("nis-S", text)
		text = nes.sub("nis", text)
		text = ment2.sub("-muhnt", text)
		text = uble1.sub("-ubl", text)
		text = uble2.sub("-ubl", text)
		text = exs.sub("X\\1", text)
		text = exm.sub("-X\\1", text)
		
		sybs = text.split("-")
		result = ""
		count = 1
		
		splits = []
		for i in sybs:
			splits = splits + [False]
		
		while count < len(sybs):
			if splits[count] == False:
				splits[count] = splitsyb(sybs[count])
			parts = splits[count]
			found = False;
			if parts:
				if  (parts[0] == "" and (parts[1] == "" or parts[1] == "u" or parts[1] == "uh")):
					if splits[count-1] == False:
						splits[count-1] = splitsyb(sybs[count-1])
					prevparts = splits[count-1]
					if prevparts:
						if not (prevparts[0]=="" and prevparts[2]=="r" and (prevparts[1]=="u" or prevparts[1]=="e")) and (parts[2] != "r") and ((prevparts[2] + parts[2]) in edict and prevparts[1] != "" and parts[2] != ""):
							if count == len(sybs)-1 and parts[2] == "":
								count = count
							else:
								sybs[count-1] = sybs[count-1] + parts[2]
								splits[count-1][2] = splits[count-1][2] + parts[2]
								sybs = sybs[:count] + sybs[count+1:]
								splits = splits[:count] + splits[count+1:]
								count = count-1
								found = True;
				if not found and parts[0] != "" and parts[2] != "" and (parts[1] == "u" or parts[1] == "uh"):
					if splits[count-1] == False:
						splits[count-1] = splitsyb(sybs[count-1])
					prevparts = splits[count-1]
					if prevparts:
						if not (prevparts[0]=="" and prevparts[2]=="r" and (prevparts[1]=="u" or prevparts[1]=="e")) and (prevparts[2] + parts[0] + parts[2]) in edict and prevparts[1] != "":
							sybs[count-1] = sybs[count-1] + parts[0] + parts[2]
							splits[count-1][2] = splits[count-1][2] + parts[0] + parts[2]
							sybs = sybs[:count] + sybs[count+1:]
							splits = splits[:count] + splits[count+1:]
							count = count-1
							found = True;
				if not found and (parts[0] != "" and parts[2] == "r") and (parts[1] == "" or parts[1] == "e" or parts[1] == "u" or parts[1] == "eh" or parts[1] == "uh"):
					if splits[count-1] == False:
						splits[count-1] = splitsyb(sybs[count-1])
					prevparts = splits[count-1]
					if prevparts:
						if not (prevparts[0]=="" and prevparts[2]=="r" and (prevparts[1]=="u" or prevparts[1]=="e")) and (prevparts[2] + parts[0]) in edict and prevparts[1] != "":
							sybs[count-1] = sybs[count-1] + parts[0]
							splits[count-1][2] = splits[count-1][2] + parts[0]
							sybs[count] = parts[1]+parts[2]
							splits[count][0] = ""
							found = True;
				if not found and parts[2] == "" and parts[0] != "" and parts[0] != "r" and(parts[1] == "ee" or parts[1] == "yee"):
					if splits[count-1] == False:
						splits[count-1] = splitsyb(sybs[count-1])
					prevparts = splits[count-1]
					if prevparts:
						if not (prevparts[0]=="" and prevparts[2]=="r" and (prevparts[1]=="u" or prevparts[1]=="e")) and (prevparts[2] + parts[0]) in edict and prevparts[1] != "":
							sybs[count-1] = sybs[count-1] + parts[0]
							splits[count-1][2] = splits[count-1][2] + parts[0]
							sybs[count] = "ee"
							splits[count][0] = ""
							found = True;
			count = count+1
		
		for i in sybs:
			result = result + "-" + i
		
		result = qsub.sub("sh", result)
		result = zsub.sub("ch", result)
		result = vsub.sub("th", result)
		
		result = result[1:]
		"""
		then we take the modified phonetic pronunciation and convert it to X padded strokes
		"""
		
		# lines = line.split("::")
		sybs = fixu.sub("u", result)
		sybs = sybs.split("-")
		fullresult = ""
		result = ""
		
		first = True
		
		for s in sybs:
			result = ""
			
			if s in fdict:
				result = fdict[s]
			
			intial = ""
			mid = ""
			final = ""
			
			syb = unify.sub("", s)
			cnt = 1
			
			if syb in fdict:
				result = fdict[syb]
			
			while cnt <= len(syb):
				if syb[0:cnt] in idict:
					intial = syb[0:cnt]
				cnt = cnt+1
			cnt = len(intial)+1
			while cnt <= len(syb):
				if syb[len(intial):cnt] in mdict:
					mid = syb[len(intial):cnt]
				cnt = cnt+1
			cnt = len(intial) + len(mid) + 1
			while cnt <= len(syb):
				if syb[len(intial) + len(mid):cnt] in edict:
					final = syb[len(intial) + len(mid):cnt]
				cnt = cnt+1
			if mid == "" and ((intial + final) in edict):
				final = intial+final
				intial = ""
			
			if (intial + mid + final) != syb and result == "":
				print "could not convert " + syb + " in " + lines[0]
			
				
			if result == "":
				if intial == "":
					result = "XXXXXXX"
					if first and lines[0].startswith("h"):
						result = idict["h"]
				elif intial in idict:
					result = idict[intial]
					if first:
						if result == idict["n"] and (lines[0].startswith("kn") or lines[0].startswith("gn")):
							result = idict["kn"]
						if result == idict["w"] and lines[0].startswith("wh"):
							result = idict["wh"]
						if result == idict["h"] and lines[0].startswith("wh"):
							result = idict["wh"]
						if result == idict["r"] and lines[0].startswith("wr"):
							result = idict["wr"]
				else:
					print "unknown intial: " + intial + " in " + s + " from " + lines[0]
					
				if mid == "":
					result = result + "XXXXX"
				elif mid in mdict:
					if mdict[mid] == "AOXXU" and "oo" in lines[0]:
						result = result + "AOXXX"
					else:
						result = result + mdict[mid]
				else:
					print "unknown middle: " + mid + " in " + s + " from " + lines[0]
				
				if final == "":
					result = result + "XXXXXXXXXX"
				elif final in edict:
					result = result + edict[final]
				else:
					print "unknown final: " + final + " in " + s + " from " + lines[0]
				
			if result != "":
				if result.endswith("*"):
					result = ast.sub("", result)
					result = result[:9]+"*"+result[10:]
				fullresult = fullresult + "\\" + result
			
			first = False
		
		#out.write(lines[0] + "::" + fullresult[1:] + "\r\n")
		if fullresult[1:] in words:
			words[fullresult[1:]] = words[fullresult[1:]] + [lines[0]]
		else:
			words[fullresult[1:]] = [lines[0]]
	line = fileI.readline()

"""
in this section an attempt to make words shorter by folding in suffixes is made
this is theory dependent
ABSOLUTELY MUST BE MODIFIED
"""

print "finished reading in and parsing file, now folding"

def combineS(n):
	full = ""
	for i in n:
		full = full + "\\" + i
	return full[1:]
	
	
for stroke in words:
	strokes = stroke.split("\\")
	
	count = 1
	if strokes[0] == "XXXXXXXAXXXXXXXXXXXXXX" or strokes[0] == "XXXXXXXXXXEXXXXXXXXXXX"  or strokes[0] == "XXXXXXXXXXXUXXXXXXXXXX":
		if len(strokes) > 0+1:
			strokes = ["S" + strokes[0+1][1:]] + strokes[2:]
			f = combineS(strokes)
			if f in replace:
				replace[f] = words[stroke] + replace[f]
			else:
				replace[f] = words[stroke]
	
	while count < len(strokes):
		if strokes[count] == "XXXXXXXAXXXXXXXXXXXXXX" or strokes[count] == "XXXXXXXXXXEXXXXXXXXXXX"  or strokes[count] == "XXXXXXXXXXXUXXXXXXXXXX":
			if len(strokes) > count+1:
				# print strokes
				strokes = strokes[:count] + ["S" + strokes[count+1][1:]] + strokes[count+2:]
				f = combineS(strokes)
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				count = count - 1
		elif strokes[count] == "XXXXXXXXXXXXXXXXXXXXXZ":
			if strokes[count-1][18:] == "XXXX" or strokes[count-1][18:] == "XSXX":
				t = strokes[count-1][:21] + "Z"
				f = combineS(strokes[:count-1] + [t] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t] + strokes[count+1:]
				count = count - 1
			elif strokes[count-1][18:] == "TXXX" or strokes[count-1][18:] == "TSXX":
				t = strokes[count-1][:20] + "DZ"
				f = combineS(strokes[:count-1] + [t] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t] + strokes[count+1:]
				count = count - 1
			elif strokes[count-1][18:] == "TXDZ":
				t = strokes[count-1][:18] + "TSDZ"
				f = combineS(strokes[:count-1] + [t] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t] + strokes[count+1:]
				count = count - 1
		elif strokes[count] == "XXXXXXXAOXEXXXXXXXXXXX" or strokes[count] == "XXXXXHRAOXEXXXXXXXXXXX":
			if strokes[count-1][18:] == "XXXX":
				t = strokes[count-1][:18] + "TXXX"
				f = combineS(strokes[:count-1] + [t] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t] + strokes[count+1:]
				count = count - 1
			elif strokes[count-1][18:] == "XSXX":
				t = strokes[count-1][:18] + "TSXX"
				f = combineS(strokes[:count-1] + [t] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t] + strokes[count+1:]
				count = count - 1
			elif strokes[count-1][18:] == "XXXZ":
				t = strokes[count-1][:18] + "TXDX"
				f = combineS(strokes[:count-1] + [t] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t] + strokes[count+1:]
				count = count - 1
		elif strokes[count] == "XXXXXXXXXXXXXXXXXXXXDZ":
			if strokes[count-1][18:] == "XXXX":
				t = strokes[count-1][:18] + "XXDZ"
				f = combineS(strokes[:count-1] + [t] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t] + strokes[count+1:]
				count = count - 1
		elif strokes[count] == "XXXXXXXXXXXXXXXXXXXSDZ":
			if strokes[count-1][18:] == "XXXX":
				t = strokes[count-1][:18] + "XXDZ"
				f = combineS(strokes[:count-1] + [t, "XXXXXXXXXXXXXXXXXXXXXZ"] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t, "XXXXXXXXXXXXXXXXXXXXXZ"] + strokes[count+1:]
			else:
				f = combineS(strokes[:count] + ["XXXXXXXXXXXXXXXXXXXXDZ", "XXXXXXXXXXXXXXXXXXXXXZ"] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count] + ["XXXXXXXXXXXXXXXXXXXXDZ", "XXXXXXXXXXXXXXXXXXXXXZ"] + strokes[count+1:]
		elif strokes[count] == "XXXXXXRAOXEXXXXXXXXXXX":
			if strokes[count-1][18:] == "XXXX" or strokes[count-1][18:] == "TXXX" or strokes[count-1][18:] == "XXXZ":
				t = strokes[count-1][:19] + "S" + strokes[count-1][20:]
				f = combineS(strokes[:count-1] + [t, "XXXXXXXAOXEXXXXXXXXXXX"] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t, "XXXXXXXAOXEXXXXXXXXXXX"] + strokes[count+1:]
				count = count - 1
		elif strokes[count] == "XXXXXXXXXXEXXXXXLGXXXX" or strokes[count] == "XXXXXXXXXXXUXXXXLGXXXX" or strokes[count] == "XXXXXXXXXXXXXXXXLGXXXX"  or strokes[count] == "XXXXXXRXXXXXXXXXXXXXXX":
			if strokes[count-1][18:] == "XXXX" or strokes[count-1][18:] == "TXXX" or strokes[count-1][18:] == "XXXZ":
				t = strokes[count-1][:19] + "S" + strokes[count-1][20:]
				f = combineS(strokes[:count-1] + [t] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t] + strokes[count+1:]
				count = count - 1
		elif strokes[count] == "XXXXXXXXXXXXXXXXXXXXDX":
			if strokes[count-1][18:] == "XXXX" or strokes[count-1][18:] == "TXXX":
				t = strokes[count-1][:20] + "DX"
				f = combineS(strokes[:count-1] + [t] + strokes[count+1:])
				if f in replace:
					replace[f] = words[stroke] + replace[f]
				else:
					replace[f] = words[stroke]
				strokes = strokes[:count-1] + [t] + strokes[count+1:]
				count = count - 1
		count = count + 1

"""
put everything back together
"""

print "finished folding, now dealing with duplicates pt 1"

for key in replace:
	if key in words:
		words[key] = words[key]+replace[key]
	else:
		words[key] = replace[key]
		
		
fileP = open("patch.txt", "r")
fileF = open("en.txt", "r")

vowels = re.compile(".......XXXXX..........")
dex = re.compile("X")
ee = re.compile("AO(\\*?)E")
aa = re.compile("A(\\*?)EU")
ii = re.compile("AO(\\*?)EU")
oo = re.compile("O(\\*?)E")
ints = re.compile("^KP[AE*UO-].*")

wordlist = {}
results = {}
patch = {}
patchv = {}
dict = {}

"""
load in the patch file
"""

line = fileP.readline()
while line:
	line = line.strip()
	lines = line.split("::")
	if len(lines) < 2:
		line = line.strip(",")
		lines = line.split(": ")
	# print lines[1].strip('"')
	patch[lines[1].strip('"')] = True
	dict[lines[1].strip('"')] = True
	patchv[lines[0].strip('"')] = lines[1].strip('"')
	# out.write(lines[0] +': ' + lines[1] + ',\r\n')
	line = fileP.readline()

"""
load in word frequency list
"""
line = fileF.readline()
while line:
	line = line.strip()
	lines = line.split()
	wordlist[lines[0].lower()] = int(lines[1])
	line = fileF.readline()

"""
convert keys from XXX filled to normal steno keys
"""


try:
	while True:
		key, value = words.popitem()
		temp = ""
		for i in key.split("\\"):
			if vowels.match(i):
				i = i[:8] + "-" + i[11:]
			temp = temp + "/" + i
			
		newkey = temp[1:]
		newkey = dex.sub("", newkey)
		
		for text in value:
			if newkey in results:
				if not (text in results[newkey]):
					results[newkey] = results[newkey] + [text]
					dict[text] = True
			else:
				results[newkey] = [text]
				dict[text] = True
				
			if newkey in patchv and not (text in patch):
				print text + ", " + newkey + " in conflict with patch for " + patchv[newkey]
except KeyError:
	print "dealing with duplicates pt 2"
	

"""
in this section we use some techniques to try to reduce duplicates based on spelling
"""


temp = {}

for key in results:
	if len(results[key]) > 1:
		total = []
		tv = [p for p in results[key] if not (p in patch)]
		
		allea = True
		someea = False
		allc = True
		somec = False
		allai = True
		someai = False
		
		for v in tv:
			if v.startswith("c") and ints.match(key) != None:
				somec = True
			else:
				allc = False
				
		
		if not allc and somec:
			for v in tv:
				if v.startswith("c") and ints.match(key) != None:
					newr = "TPH"+key[2:]
					# print v + ", " + newr
					if not (newr in results):
						if newr in temp:
							temp[newr] = temp[newr] + [v]
						else:
							temp[newr] = [v]
						total = total + [v]
			results[key] = [p for p in results[key] if not (p in total)]
		
		if len(results[key]) > 1:
			total = []
			tv = [p for p in results[key] if not (p in patch)]
			allsc = True
			somesc = False
			for v in tv:
				if v.startswith("sc") and ints.match(key) != None:
					somesc = True
				elif v.startswith("s") and ints.match(key) != None:
					allsc = False
			
			if not allsc and somesc:
				for v in tv:
					if v.startswith("sc") and ints.match(key) != None:
						newr = "TPH"+key[2:]
						# print v + ", " + newr
						if not (newr in results):
							if newr in temp:
								temp[newr] = temp[newr] + [v]
							else:
								temp[newr] = [v]
							total = total + [v]
				results[key] = [p for p in results[key] if not (p in total)]
		
		if len(results[key]) > 1:
			total = []
			tv = [p for p in results[key] if not (p in patch)]
			for v in tv:
				if v.find("ea") != -1:
					someea = True
				else:
					allea = False
					
			if not allea and someea:
				for v in tv:
					if v.find("ea") != -1:
						newr = ee.sub("A\\1E", key)
						newr = aa.sub("A\\1E", newr)
						# print v + ", " + newr
						if not (newr in results):
							if newr in temp:
								temp[newr] = temp[newr] + [v]
							else:
								temp[newr] = [v]
							total = total + [v]
				results[key] = [p for p in results[key] if not (p in total)]
				
		if len(results[key]) > 1:
			total = []
			tv = [p for p in results[key] if not (p in patch)]
			for v in tv:
				if v.find("ai") != -1:
					someai = True
				else:
					allai = False
			
			if not allai and someai:
				for v in tv:
					if v.find("ai") != -1:
						newr = aa.sub("AO\\1", key)
						# print v + ", " + newr
						if not (newr in results):
							if newr in temp:
								temp[newr] = temp[newr] + [v]
							else:
								temp[newr] = [v]
							total = total + [v]
							
				results[key] = [p for p in results[key] if not (p in total)]
		allia = True
		someia = False
		if len(results[key]) > 1:
			total = []
			tv = [p for p in results[key] if not (p in patch)]
			for v in tv:
				if v.find("ia") != -1:
					someia = True
				else:
					allia = False
			
			if not allia and someia:
				for v in tv:
					if v.find("ia") != -1:
						newr = ii.sub("A\\1E", key)
						# print v + ", " + newr
						if not (newr in results):
							if newr in temp:
								temp[newr] = temp[newr] + [v]
							else:
								temp[newr] = [v]
							total = total + [v]
							
				results[key] = [p for p in results[key] if not (p in total)]
				
		allei = True
		someei = False
		if len(results[key]) > 1:
			total = []
			tv = [p for p in results[key] if not (p in patch)]
			for v in tv:
				if v.find("ei") != -1:
					someei = True
				else:
					allei = False
			
			if not allei and someei:
				for v in tv:
					if v.find("ei") != -1:
						newr = aa.sub("A\\1E", key)
						# print v + ", " + newr
						if not (newr in results):
							if newr in temp:
								temp[newr] = temp[newr] + [v]
							else:
								temp[newr] = [v]
							total = total + [v]
							
				results[key] = [p for p in results[key] if not (p in total)]
				
		alloa = True
		someoa = False
		if len(results[key]) > 1:
			total = []
			tv = [p for p in results[key] if not (p in patch)]
			for v in tv:
				if v.find("oa") != -1 or v.find("oe") != -1:
					someoa = True
				else:
					alloa = False
			
			if not alloa and someoa:
				for v in tv:
					if v.find("oa") != -1 or v.find("oe") != -1:
						newr = oo.sub("AO\\1", key)
						# print v + ", " + newr
						if not (newr in results):
							if newr in temp:
								temp[newr] = temp[newr] + [v]
							else:
								temp[newr] = [v]
							total = total + [v]
							
				results[key] = [p for p in results[key] if not (p in total)]

for k in temp:
	results[k] = temp[k]


"""
in this section we output the results to the json dile
this is also were duplictes have S-P appended to them to prevent conflicts
"""

print "generating .json"

def wordvalue(n):
	if n.lower() in wordlist:
		return wordlist[n.lower()]
	else:
		return 0
	
out = open(sys.argv[1] + ".json", 'w+b')

out.write('{\r\n')

for key in patchv:
	out.write('"' + key + '": "' + patchv[key] + '",\r\n')
	
for key, value in sorted(results.iteritems(), key=operator.itemgetter(0), reverse=False):
	if len(value) == 1:
		if not (value[0] in patch):
			out.write('"' + key + '": "' + value[0] + '",\r\n')
	else:
		svalue = [p for p in value if not (p in patch)]
		svalue = sorted(svalue, key=wordvalue, reverse=True)
		i = 0
		# lastsyb = "/" + key.split("/")[-1]
		lastsyb = "/S-P"
		for n in svalue:
			if i != 0:
				while (key + (lastsyb*i)) in results:
					i = i+1
			out.write('"' + key + (lastsyb*i) +'": "' + n + '",\r\n')
			i = i+1

out.write('"O*EU/O*EU/O*EU": "Put your copyright notice here"\r\n')
out.write('}')