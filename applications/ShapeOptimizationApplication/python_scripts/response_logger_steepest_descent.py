# ==============================================================================
#  KratosShapeOptimizationApplication
#
#  License:         BSD License
#                   license: ShapeOptimizationApplication/license.txt
#
#  Main authors:    Baumgaertner Daniel, https://github.com/dbaumgaertner
#                   Geiser Armin, https://github.com/armingeiser
#
# ==============================================================================

# Making KratosMultiphysics backward compatible with python 2.6 and 2.7
from __future__ import print_function, absolute_import, division

# importing the Kratos Library
from KratosMultiphysics import *
from KratosMultiphysics.ShapeOptimizationApplication import *

# check that KratosMultiphysics was imported in the main script
CheckForPreviousImport()

# Import logger base classes
from response_logger_base import ResponseLogger

# Import additional libraries
import csv

# ==============================================================================
class ResponseLoggerSteepestDescent( ResponseLogger ):

    # --------------------------------------------------------------------------
    def __init__( self, communicator, optimizationSettings, timer ):
        self.communicator = communicator
        self.optimizationSettings = optimizationSettings
        self.timer = timer

        self.onlyObjective = self.optimizationSettings["objectives"][0]["identifier"].GetString()   

        self.completeResponseLogFileName = self.__CreateCompleteResponseLogFilename( optimizationSettings )

        self.objectiveValueHistory = {}
        self.objectiveReferenceValue = None        
        self.absoluteChangeOfObjectiveValueHistory = {}
        self.relativeChangeOfObjectiveValueHistory = {}

        self.currentOptimizationIteration = 0
        self.previousOptimizationIteration = 0
        self.initialOptimizationIteration = 0

    # --------------------------------------------------------------------------
    def __CreateCompleteResponseLogFilename( self, optimizationSettings ):
        resultsDirectory = optimizationSettings["output"]["output_directory"].GetString()
        responseLogFilename = optimizationSettings["output"]["response_log_filename"].GetString()
        completeResponseLogFilename = resultsDirectory+"/"+responseLogFilename+".csv"
        return completeResponseLogFilename     

    # --------------------------------------------------------------------------
    def InitializeLogging( self ):
        with open(self.completeResponseLogFileName, 'w') as csvfile:
            historyWriter = csv.writer(csvfile, delimiter=',',quotechar='|',quoting=csv.QUOTE_MINIMAL)
            row = []
            row.append("{:<6s}".format("itr"))
            row.append("{:>20s}".format("f"))
            row.append("{:>12s}".format("df_abs[%]"))
            row.append("{:>12s}".format("df_rel[%]"))
            row.append("{:>13s}".format("step_size[-]"))
            row.append("{:>12s}".format("t_itr[s]"))
            row.append("{:>16s}".format("t_total[s]"))
            row.append("{:>25s}".format("time_stamp"))
            historyWriter.writerow(row)  

    # --------------------------------------------------------------------------
    def LogCurrentResponses( self, optimizationIteration ):
        self.currentOptimizationIteration = optimizationIteration

        if self.__IsFirstLog():
            self.initialOptimizationIteration = optimizationIteration        
            self.__AddObjectiveValueToHistory()
            self.__DetermineObjectiveReferenceValue()
            self.__InitializeChangeOfObjectiveValueHistory()
        else:
            self.__AddObjectiveValueToHistory()
            self.__DetermineObjectiveReferenceValue()
            self.__AddChangeOfObjectiveValueToHistory()
        self.__PrintInfoAboutResponseFunctionValues()
        self.__WriteDataToLogFile()
        
        self.previousOptimizationIteration = optimizationIteration

    # -------------------------------------------------------------------------
    def __IsFirstLog( self ):
        if len(self.objectiveValueHistory) == 0:
            return True
        else:
            return False  

    # --------------------------------------------------------------------------
    def __AddObjectiveValueToHistory( self ):
        objectiveValue = self.communicator.getReportedFunctionValueOf ( self.onlyObjective )
        self.objectiveValueHistory[self.currentOptimizationIteration] = objectiveValue

    # --------------------------------------------------------------------------
    def __DetermineObjectiveReferenceValue( self ):
        self.objectiveReferenceValue = self.communicator.getReportedFunctionReferenceValueOf ( self.onlyObjective )
        if not self.objectiveReferenceValue:
            self.objectiveReferenceValue = self.objectiveValueHistory[self.initialOptimizationIteration]
        if abs(self.objectiveReferenceValue)<1e-12:
            print("\n> WARNING: Objective reference value < 1e-12!!:")
            print("> WARNING: I.e. either initial objective value is zero and no reference value is specified in the analyzer or specified reference value is zero.")
            print("> WARNING: Standard reference value of 1 is assumed.")
            self.objectiveReferenceValue = 1.0     

    # --------------------------------------------------------------------------
    def __InitializeChangeOfObjectiveValueHistory( self ):
        self.absoluteChangeOfObjectiveValueHistory[self.currentOptimizationIteration] = 0.0
        self.relativeChangeOfObjectiveValueHistory[self.currentOptimizationIteration] = 0.0

    # --------------------------------------------------------------------------
    def __AddChangeOfObjectiveValueToHistory( self ):
        objectiveValue = self.objectiveValueHistory[self.currentOptimizationIteration]
        previousObjectiveValue = self.objectiveValueHistory[self.previousOptimizationIteration]
        initialObjectiveValue = self.objectiveValueHistory[self.initialOptimizationIteration]

        self.absoluteChangeOfObjectiveValueHistory[self.currentOptimizationIteration] = 100*(objectiveValue-initialObjectiveValue) / self.objectiveReferenceValue
        self.relativeChangeOfObjectiveValueHistory[self.currentOptimizationIteration] = 100*(objectiveValue-previousObjectiveValue) / self.objectiveReferenceValue

    # --------------------------------------------------------------------------
    def __PrintInfoAboutResponseFunctionValues( self ):
        objectiveValue = self.objectiveValueHistory[self.currentOptimizationIteration]
        absoluteChangeOfObjectiveValue = self.absoluteChangeOfObjectiveValueHistory[self.currentOptimizationIteration]
        relativeChangeOfObjectiveValue = self.relativeChangeOfObjectiveValueHistory[self.currentOptimizationIteration]        
        print("\n> Current value of objective function = ", round(objectiveValue,12))
        print("> Absolut change of objective function = ",round(absoluteChangeOfObjectiveValue,4)," [%]")
        print("> Relative change of objective function = ",round(relativeChangeOfObjectiveValue,4)," [%]")         

    # --------------------------------------------------------------------------
    def __WriteDataToLogFile( self ):
        objectiveValue = self.objectiveValueHistory[self.currentOptimizationIteration]

        absoluteChangeOfObjectiveValue = self.absoluteChangeOfObjectiveValueHistory[self.currentOptimizationIteration]
        relativeChangeOfObjectiveValue = self.relativeChangeOfObjectiveValueHistory[self.currentOptimizationIteration]

        with open(self.completeResponseLogFileName, 'a') as csvfile:
            historyWriter = csv.writer(csvfile, delimiter=',',quotechar='|',quoting=csv.QUOTE_MINIMAL)
            row = []
            row.append("{:<6s}".format(str(self.currentOptimizationIteration)))
            row.append(str("{:>20f}".format(objectiveValue)))
            row.append(str("{:>12f}".format(absoluteChangeOfObjectiveValue)))
            row.append(str("{:>12f}".format(relativeChangeOfObjectiveValue)))
            row.append(str("{:>13f}".format(self.optimizationSettings["line_search"]["step_size"].GetDouble())))
            row.append(str("{:>12f}".format(self.timer.GetLapTime())))
            row.append(str("{:>16f}".format(self.timer.GetTotalTime())))
            row.append("{:>25}".format(self.timer.GetTimeStamp()))
            historyWriter.writerow(row)

    # --------------------------------------------------------------------------
    def FinalizeLogging( self ):      
        pass # No finalization necessary here

    # --------------------------------------------------------------------------
    def GetValue( self, variableKey ):
        
        if variableKey=="RELATIVE_CHANGE_OF_OBJECTIVE_VALUE":
            if self.__IsFirstLog():
                raise RuntimeError("Relative change of objective function can not be computed since only one logged value is existing!")
            else:
                return self.relativeChangeOfObjectiveValueHistory[self.currentOptimizationIteration]
        else:
            raise NameError("Value with the following variable key not defined in response_logger_penalized_projection.py: " + variableKey)

# ==============================================================================