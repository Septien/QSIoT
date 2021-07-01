"""
Plot the data corresponding to the performance of the KEMs.
The performance data available corresponds to the following variables:
    -CPU cycles and time.
    -Memory usage.
    -Packet size.

The data for the CPU usage is at:
    -CPUPerformance/cyclesCPUPerformance.csv
    -CPUPerformance/timeCPUPerformance.csv

The data for the memory usage is at:
    -memoryPerformance/memoryPerformance.csv

The data for the packet size is at:
    -packetsPerformance/packetPerformance.csv

The CPU and packet performance files, have the following format:
    -Fields.
    -KEM.
    -Data.
The firs row contains the name of the fields measured, for each variable.
Then comes the name of the KEM for all of them, and finally the data 
associated to the field and the KEM. The first entry of the "Fields" row 
corresponds to the first row of the Data section, and so on.

The file containing data about the memory has a similar format:
    -KEM.
    -Data.
The KEM row indicates the name of the KEM under measure, and the Data section
contains the data. The first row corresponds to the first KEM, and so on. The
units are in bytes.
"""

import numpy as np
import csv
import matplotlib.pyplot as plt
import pandas as pd

def loadDataCPUPerformance(file, delimiter, nKEM):
    """
    Load the data for CPU performance following the established format.
    Return a matrix of size (3 * m) * N, where m is the number of kems, and 
    N is the number of executions; the name of the fields; and the units.
    """
    fields = []
    unit = []
    kem = []
    data = []
    with open(file, 'r') as f:
        reader = csv.reader(f, delimiter=delimiter)
        # Get the fields and units
        r = next(reader)
        fields = [r[0].split(" ")[0], r[1].split(" ")[0], r[2].split(" ")[0]]
        unit = r[0].split(" ")[1]
        for i in range(nKEM):
            # Get the name of the KEM
            kem.append(next(reader)[0])
            # Get the data
            row = next(reader)
            data.append([float(r) for r in row[:-1]])
            row = next(reader)
            data.append([float(r) for r in row[:-1]])
            row = next(reader)
            data.append([float(r) for r in row[:-1]])
    return fields, unit, kem, np.array(data, dtype=object)

def loadDataMemory(file, delimiter):
    """
    Loads data of memory performance for each of the KEMs.
    Returns a 5 * N matrix, where N is the number of executions.
    """
    data = []
    kem = []
    with open(file, 'r') as f:
        reader = csv.reader(f, delimiter=delimiter)
        kem = next(reader)
        maxLen = 0
        for row in reader:
            if len(row) > maxLen:
                maxLen = len(row)
            d = [int(r) for r in row]
            data.append(d)
        maxLen += 1
        # Normalize the length of the rows
        for i in range(len(data)):
            for j in range(len(data[i]), maxLen):
                data[i].append(0)
    return kem, np.array(data, dtype=object)
        
def loadDataPacket(file, delimiter):
    """
    Loads the data corresponding to the packet performance.
    We currently are only interestedi in the following variables:
        -Number of bytes transmitted.
        -Duration of the connection.
    Returns the name of the fields, the name of the KEMs, and a 
    matrix of size (m * 3) * N, where m is the number of KEMS, 
    and N the number of runs.
    """
    kemData = []
    kem = []
    fields = []
    with open(file, 'r') as f:
        reader = csv.reader(f, delimiter=delimiter)
        # Get the field information
        row = next(reader)
        fields = [row[0], row[1], row[6]]
        i = 0
        for row in reader:
            # Get the KEMs name
            if i % 10 == 0:
                kem.append(row[0])
            # Get the number of packets transmitted
            if i % 10 == 1:
                d = [int(r) for r in row]
                kemData.append(d)
            # Get the number of bytes
            if i % 10 == 2:
                d = [int(r) for r in row]
                kemData.append(d)
            # Get the duration of the connection
            if i % 10 == 7:
                d = [float(r) for r in row]
                kemData.append(d)
            
            i += 1
    return kem, fields, np.array(kemData, dtype=object)

def computeStatistics(data, pkt=False):
    """
    Computes the statistics for each variable of interest. As each 
    variable has different fields, and for each field the statistics 
    are computed, it is needed to indicate the current variable.
    Returns an array with statistics in the following order:
        -Mean.
        -Maximum.
        -Standard Deviation.
        -Variance.
    """
    statistics = []
    for row in data:
        st = [np.mean(row), np.amax(row), np.std(row), np.var(row)]
        statistics.append(st.copy())
    return np.array(statistics)


def saveStatistics(filename, delimiter, kems, data, fields=None):
    """
    Save the statistics data on a file called 'filename', a csv file.
    On the first row, the name of the kems will be stored. If the variable
    has fields, the following road contains the name of them. On the
    rest of the file, the statistics data are stored.
    """
    with open(filename, 'w') as file:
        writer = csv.writer(file, delimiter=delimiter)
        writer.writerow(kems)
        if fields:
            writer.writerow(fields)
        writer.writerows(data)

def barGraph(dictionary, kems, units, imageName, statisticName, logy):
    """
    Uses matplotlib to plot data on a graph bar.
    https://markhneedham.com/blog/2018/09/18/matplotlib-remove-axis-legend/
    https://stackoverflow.com/questions/30228069/how-to-display-the-value-of-the-bar-on-each-bar-with-pyplot-barh
    https://www.reddit.com/r/learnpython/comments/9l948p/having_a_bit_of_trouble_sorting_bars_in/
    """
    # Plot the performance of each cipher with a logarithmic scale
    dfTFastest = pd.DataFrame(dictionary, index=kems)
    fig, ax = plt.subplots()
    dfTFastest.plot(kind="bar", ax=ax, rot=45, grid=True, logy=logy)
    ax.set_axisbelow(True)
    plt.ylabel(units)
    plt.title(statisticName)
    plt.tight_layout()
    plt.savefig(imageName + ".svg")
    plt.close()

def plotStatisticsOnBarGraph(statistics, statisticsNames, fields, variable, kems, imageName, units, logy=False, byField=False):
    """
    Plot all the statistics on a bar graph, and save the image to 'imageName'.
    Inputs:
        -statistics: array containing the statistics.
        -statisticsName: name of each statistic.
        -fields: array containing the name of the fields for each variable.
        -variable: name of the variable under study.
        -kems: name of the kem under study.
        -imageName: name of the image to save.
        -logy: logarithmic scale on y-axis?
    Group by field, if necessary
    """
    for i in range(len(statisticsNames)):
        df = {}
        # Get the ith statistics
        ithSt = statistics[:, i]
        # Group by field
        nFields = len(fields)
        for j in range(nFields):
            fieldStatistics = []
            for k in range(len(kems)):
                fieldStatistics.append(ithSt[j + (k * nFields)])
            df[fields[j]] = fieldStatistics.copy()
        if byField:
            for j in range(len(fields)):
                barGraph({fields[j]: df[fields[j]]}, kems, units[j], imageName + statisticsNames[i] + fields[j], statisticsNames[i], logy)
        else:
            barGraph(df, kems, units, imageName + statisticsNames[i], statisticsNames[i], logy)

def linePlot(data, unit, fieldName, kems, imageName, logy=False):
    """
    Plot the data on a line graph.
        -data: Arrays containing the data per field.
        -unit: Unit of the field.
        -field: Name of the field.
        -kems: Name of the kems.
        -imageName: Where to save the image.
    """
    # Create a dictionary with the data
    dictD = {}
    for i in range(len(data)):
        dictD[kems[i]] = data[i]
    df = pd.DataFrame(dictD, index=range(len(data[0])))
    fig, ax = plt.subplots()
    df.plot(kind="line", ax=ax, rot=0, grid=True, logy=logy)
    plt.title(fieldName)
    plt.ylabel(unit)
    plt.xlabel("Iteration")
    plt.tight_layout()
    plt.savefig(imageName + unit + ".svg")
    plt.close()

def plotDataOnLinePlot(data, units, fields, kems, imageName, logy=False):
    """
    Separates the data by fields, and then pass it to a function to plot all the
    data on a single graph.
        -data: Arrays containing the performance data.
        -units: Units of each field.
        -fields: Name of the fields.
        -kems: Name of the KEMs.
        -imageName: Where to save the image.
    """
    # Get the number of fields
    nFields = len(fields)
    # Separate the data on fields
    for i in range(nFields):
        dataByFields = []
        for j in range(len(kem)):
            dataByFields.append(data[i + (nFields * j)])
        linePlot(dataByFields, units[i], fields[i], kems, imageName + fields[i], logy)

def plotDataOnBoxPlot(data, fields, kems, unit, imageName, logy=False):
    """
    Group all the data on by kems, and then plot it on a box plot, on a single graph.
        -data: Arrays containing the data.
        -units: Units of each field.
        -fields: Name of the fields.
        -kems: Name of the KEMs.
        -imageName: Where to save the image.
    """
    kem1 = [data[0], data[1], data[2]]
    kem2 = [data[3], data[4], data[5]]
    kem3 = [data[6], data[7], data[8]]
    kem4 = [data[9], data[10], data[11]]
    kem5 = [data[12], data[13], data[14]]

    #bpk1 = plt.boxplot(kem1, positions=[0,1,2], sym='', widths=0.6)
    #bpk2 = plt.boxplot(kem2, positions=[3,4,5], sym='', widths=0.6)
    bpk3 = plt.boxplot(kem3, positions=[6,7,8], sym='', widths=0.6)
    bpk4 = plt.boxplot(kem4, positions=[9,10,11], sym='', widths=0.6)
    #bpk5 = plt.boxplot(kem5, positions=[12,13,14], sym='', widths=0.6)

    #plt.plot([], c='blue', label=kem[0])
    #plt.plot([], c='red', label=kem[1])
    plt.plot([], c='green', label=kem[2])
    plt.plot([], c='yellow', label=kem[3])
    #plt.plot([], c='magenta', label=kem[4])

    plt.xticks([1, 4, 6], fields)
    #plt.yscale('log')
    plt.tight_layout()
    plt.savefig(imageName)
    plt.close()

if __name__ == '__main__':
    stats = ["Mean", "Maximum", "Standard Deviation", "Variance"]
    # For CPU performance
    fields, unit, kem, data = loadDataCPUPerformance("CPUPerformance/timeCPUPerformance.csv", ',', 5)
    statistics = computeStatistics(data)
    plotStatisticsOnBarGraph(statistics, stats, fields, "CPU", kem, "images/cpuPerformanceRPI", "milliseconds", True)
    plotDataOnLinePlot(data, ["milliseconds", "milliseconds", "milliseconds"], fields, kem, "images/cpuUsageRPI", True)
    plotDataOnBoxPlot(data, fields, kem, unit, "images/cpuBehaviourRPI.svg", True)
    saveStatistics("statistics/cpuStatRPI.csv", ',', kem, statistics, fields)

    # For memory performance
    kem, data = loadDataMemory("memoryPerformance/memoryPerformance.csv", ',')
    statistics = computeStatistics(data)
    plotStatisticsOnBarGraph(statistics, stats, ["Memory"], "Memory", kem, "images/memoryPerformanceRPI", "Bytes", True)
    plotDataOnLinePlot(data, ["Bytes"], ["Memory access"], kem, "images/memoryUsageRPI", True)
    saveStatistics("statistics/memoryStatRPI.csv", ',', kem, statistics)
    
    # # For packet performance
    kem, fields, kemData = loadDataPacket("packetsPerformance/packetPerformance.csv", ',')
    statistics = computeStatistics(kemData)
    plotStatisticsOnBarGraph(statistics, stats, fields, "Packets", kem, "images/packetPerformanceRPI", ["Packets", "Bytes", "mSec"], False, True)
    plotDataOnLinePlot(kemData, ["Packets", "Bytes", "mSec"], fields, kem, "images/packetUsageRPI")
    saveStatistics("statistics/packetStatRPI.csv", ',', kem, statistics, fields)
