from Bjontegaard import *
from Utilities import *
import PSetBuilder
import Configuration
import os
#from joblib import Parallel, delayed  

psetFileOut = open('PSET_results.csv','w')
print >> psetFileOut, 'Params\tTime Savings\tBD-BR Inc\tRDCCOst'
psetFileOut.close()
for gopStructure in Configuration.gopStructureList:

	for [sequence, numFrames] in Configuration.sequenceList:

		refBDResults = []
		refTimeResults = []
		optParams = Configuration.optParamsRef
		pathToBin = Configuration.pathToRefBin

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
				[bd, time] = parsed
			else:
				[bd, time] = [['N/A'], 'N/A']

			refBDResults.append(bd)
			refTimeResults.append(time)

	currCfg = ''
	costList = PSetBuilder.BDRateCostList
	while not(PSetBuilder.allTargetsCovered(costList)):
		avgTimeSavings = 0.0
		avgBdrateIncY = 0.0

		testBDResults = []
		testTimeResults = []

		testName = PSetBuilder.getExtraCfg()
		if testName:
			currCfg = PSetBuilder.getCmdString(testName)
		else:
			if PSetBuilder.psetAlgorithm == 'greedy':
				currCfg = PSetBuilder.getNextCfgGreedy(currCfg, costList)
			elif PSetBuilder.psetAlgorithm == 'greedy_v2':
				currCfg = PSetBuilder.getNextCfgGreedy_v2(costList)
			elif PSetBuilder.psetAlgorithm == 'greedy_v3':
				currCfg = PSetBuilder.getNextCfgGreedy_v3(costList)
			else:
				currCfg = PSetBuilder.getNextCfg(currCfg, costList)

			if not(currCfg):
				break
			testName = PSetBuilder.getCfgString(currCfg)

		

		invalidSeqs = 0
		for [sequence, numFrames] in Configuration.sequenceList:	
			timeSavings = 0.0
			bdrateIncY = 0.0
			pathToBin = Configuration.pathToRefBin

			if Configuration.RUN_TEST:
				if Configuration.RUN_PARALLEL:
					Parallel(n_jobs=Configuration.NUM_THREADS)(delayed(runParallelSims)(sequence,numFrames, gopStructure, qp, pathToBin, currCfg, 'test', testName) for qp in Configuration.qpList)	
				else:
					for qp in Configuration.qpList:
						runParallelSims(sequence,numFrames, gopStructure, qp, pathToBin, currCfg, 'test', testName)

			qpBDResults = []
			qpTimeResults = []
			for qp in Configuration.qpList:
				[gopPath, seqPath, resultsPath] = treatConfig(sequence, gopStructure, qp, 'test', testName)
				parsed = parseOutput(resultsPath)
				if parsed:
					[bd, time] = parsed
				else:
					[bd, time] = [['N/A'], 'N/A']
				qpBDResults.append(bd)
				qpTimeResults.append(time)
			
#			testBDResults.append(qpBDResults)
#			testTimeResults.append(qpTimeResults)
			testBDResults = qpBDResults
			testTimeResults = qpTimeResults
			if time != 'N/A':
				timeSavings = 1.0-(sum(testTimeResults)/sum(refTimeResults))
				avgTimeSavings += timeSavings
				bdrateIncY = bdrate(refBDResults, testBDResults, 1)/100
				bdrateIncU = bdrate(refBDResults, testBDResults, 2)/100
				bdrateIncV = bdrate(refBDResults, testBDResults, 3)/100
				avgBdrateIncY += bdrateIncY
			else:
				invalidSeqs+=1


		try:
			avgTimeSavings /= (len(Configuration.sequenceList)-invalidSeqs)*1.0
			if avgTimeSavings <= 0: avgTimeSavings = 0.0001
			avgBdrateIncY /= (len(Configuration.sequenceList)-invalidSeqs)*1.0
			if avgBdrateIncY <= 0: avgBdrateIncY = 0
			RDCompCost = float(avgBdrateIncY/avgTimeSavings)
		except:
			continue
		
		if PSetBuilder.psetAlgorithm != 'greedy':
			costList[testName] = [avgBdrateIncY, avgTimeSavings, RDCompCost]
		psetFileOut = open('PSET_results.csv','a')	
		print >> psetFileOut, '%s\t%.4f\t%.4f\t%.4f' % (testName, avgTimeSavings, avgBdrateIncY, RDCompCost)
		psetFileOut.close()

		#print '%s\t%.4f\t%.4f\t%.4f' % (testName, avgTimeSavings, avgBdrateIncY, RDCompCost)


