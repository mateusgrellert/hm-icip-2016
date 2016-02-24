
def buildTestSet(path):
	inputFile = open(path, 'r')
	binList = []
	paramList = []
	testNameList = []
	for line in inputFile.readlines():
		if '#' in line: continue

		line = line.split()
		name = line[0]
		bin = line[1]
		params = ' --'.join(line[2:])
		binList.append(bin)
		testNameList.append(name)
		if params:
			paramList.append(' --'+params)
		else:
			paramList.append(' ')
	inputFile.close()
	
	return [testNameList, binList, paramList]


gopStructureList = ['encoder_lowdelay_main']
sequenceList = [['BQMall', 32]] #, \
"""				['Traffic', 64],['PeopleOnStreet', 64], \
				['BasketballDrive', 64],['ParkScene', 64], \
				['RaceHorsesC', 64],['BQMall', 64], \
				['ChinaSpeed', 64],['BasketballDrillText', 64], \
				['Johnny', 64],['FourPeople', 64]]"""
qpList = ['22', '27', '32', '37']

cfgPath = '../cfg'
sequencePath = '/Users/grellert/hm-cfgs/cropped'
hmOutputPath = './hmoutput_bqmall_32f_blacky'

pathToRefBin = '../bin/TAppEncoderStatic'
optParamsRef = ''

#[testNameList, pathToTestBinList, optParamsTestList] = buildTestSet('Tests.inp')

#N_TESTS = len(pathToTestBinList)
RUN_REFERENCE = True
RUN_TEST = True
RUN_PARALLEL = True
NUM_THREADS = 4
