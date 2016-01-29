from Bjontegaard import *
from Utilities import *
import Configuration
import os
from joblib import Parallel, delayed  


for gopStructure in Configuration.gopStructureList:
	bdRateFile = makeBDRateFile(gopStructure)
	rdValuesFile = makeRDValuesFile(gopStructure)
	for [sequence, numFrames] in Configuration.sequenceList:
		print >> bdRateFile, sequence,

		refBDResults = []
		refTimeResults = []
		optParams = Configuration.optParamsRef
		pathToBin = Configuration.pathToRefBin
		rdFileLine = {}
		if Configuration.RUN_REFERENCE:
			if Configuration.RUN_PARALLEL:
				Parallel(n_jobs=Configuration.NUM_THREADS)(delayed(runParallelSims)(sequence,numFrames, gopStructure, qp, pathToBin, optParams, 'ref') for qp in Configuration.qpList)
			else:
				for qp in Configuration.qpList:
					runParallelSims(sequence,numFrames, gopStructure, qp, pathToBin, optParams, 'ref') 

		for qp in Configuration.qpList:
			[gopPath, seqPath, resultsPath] = treatConfig(sequence, gopStructure, qp, 'ref')
			parsed = parseOutput(resultsPath)
			if parsed:
				[bd, time] = parseOutput(resultsPath)
			else:
				[bd, time] = [['N/A'], 'N/A']
			rdFileLine[qp] = '\t'.join([str(x) for x in [sequence]+[qp]+bd+[time]]) + '\t'

			refBDResults.append(bd)
			refTimeResults.append(time)
		

		for testIdx in range(Configuration.N_TESTS):
			testBDResults = []
			testTimeResults = []
			optParams = Configuration.optParamsTestList[testIdx]
			pathToBin = Configuration.pathToTestBinList[testIdx]
			testName = Configuration.testNameList[testIdx]
			if Configuration.RUN_TEST:
				if Configuration.RUN_PARALLEL:
					Parallel(n_jobs=Configuration.NUM_THREADS)(delayed(runParallelSims)(sequence,numFrames, gopStructure, qp, pathToBin, optParams, 'test', testIdx) for qp in Configuration.qpList)	
				else:
					for qp in Configuration.qpList:
						runParallelSims(sequence,numFrames, gopStructure, qp, pathToBin, optParams, 'test', testIdx)


			for qp in Configuration.qpList:
				[gopPath, seqPath, resultsPath] = treatConfig(sequence, gopStructure, qp, 'test', testIdx)
				parsed = parseOutput(resultsPath)
				if parsed:
					[bd, time] = parseOutput(resultsPath)
				else:
					[bd, time] = [['N/A'], 'N/A']
				rdFileLine[qp] += '\t'.join([str(x) for x in bd+[time]]) + '\t'
				testBDResults.append(bd)
				testTimeResults.append(time)

			calcAndPrintBDRate(refTimeResults,refBDResults,testTimeResults,testBDResults, bdRateFile)
		for qp in Configuration.qpList:
			print >> rdValuesFile, rdFileLine[qp]

		print >> bdRateFile, '\n',

	bdRateFile.close()
	rdValuesFile.close()
