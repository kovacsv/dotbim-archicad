import os
import json
import urllib.request

host = 'http://127.0.0.1'
port = '19723'

def RunCommand (command, parameters):
    connectionObject = urllib.request.Request ('{}:{}'.format (host, port))
    connectionObject.add_header ('Content-Type', 'application/json')
    requestData = {
        'command' : command,
        'parameters': parameters
    }
    requestString = json.dumps (requestData).encode ('utf8')
    responseData = urllib.request.urlopen (connectionObject, requestString)
    return json.loads (responseData.read ())

def RunAddOnCommand (namespace, command, parameters):
    return RunCommand ('API.ExecuteAddOnCommand', {
        'addOnCommandId': {
            'commandNamespace': namespace,
            'commandName': command
        },
        'addOnCommandParameters': parameters
    })

desktopPath = os.path.expanduser ('~/Desktop')
dotbimFilePath = os.path.join (desktopPath, 'test.bim')

exportResult = RunAddOnCommand ('Bimdots', 'ExportDotbimFile', {
    'filePath': dotbimFilePath
})
print (exportResult)

importResult = RunAddOnCommand ('Bimdots', 'ImportDotbimFile', {
    'filePath': dotbimFilePath
})
print (importResult)
