import os
import argparse
import subprocess
import platform
import sys

def PrintError (msg):
    print ('ERROR: ' + msg)

# Parse arguments

parser = argparse.ArgumentParser ()
parser.add_argument ('--acVersion', dest = 'acVersion', type = str, required = True, help = 'Version of the target Archicad.')
parser.add_argument ('--devKitDir', dest = 'devKitDir', type = str, required = True, help = 'Location of the Archicad Development Kit.')
parser.add_argument ('--developerId', dest = 'developerId', type = str, required = False, help = 'Archicad Developer ID.')
parser.add_argument ('--localId', dest = 'localId', type = str, required = False, help = 'Add-On Local ID.')
parser.add_argument ('--projGenerator', dest = 'projGenerator', type = str, required = False, help = 'Custom cmake project generator.')
args = parser.parse_args ()

currentPath = os.path.dirname (os.path.abspath (__file__))
rootPath = os.path.dirname (currentPath)
os.chdir (rootPath)

# Generate project

generatorParams = None
if platform.system () == 'Windows':
    if args.projGenerator != None:
        generatorParams = [
            '-G', args.projGenerator,
            '-A', 'x64'
        ]
    else:
        if args.acVersion == '24':
            generatorParams = [
                '-G', 'Visual Studio 16 2019',
                '-A', 'x64',
                '-T', 'v140'
            ]
        elif args.acVersion == '25':
            generatorParams = [
                '-G', 'Visual Studio 16 2019',
                '-A', 'x64',
                '-T', 'v141'
            ]
        elif args.acVersion == '26':
            generatorParams = [
                '-G', 'Visual Studio 16 2019',
                '-A', 'x64',
                '-T', 'v142'
            ]
elif platform.system () == 'Darwin':
    if args.projGenerator != None:
        generatorParams = [
            '-G', args.projGenerator,
            '-A', 'x86_64'
        ]
    else:
        generatorParams = [
            '-G', 'Xcode',
            '-DCMAKE_OSX_ARCHITECTURES="x86_64"'
        ]

if generatorParams == None:
    PrintError ('Failed to build on this platform.')
    sys.exit (1)

buildDir = os.path.join (rootPath, 'Build', 'AC' + args.acVersion)
devKitDir = os.path.realpath (args.devKitDir)
cmake = 'cmake'
cmakeParams = []
cmakeParams.append ('cmake')
cmakeParams.extend (['-B', buildDir])
cmakeParams.extend (generatorParams)
cmakeParams.append ('-DAC_API_DEVKIT_DIR=' + devKitDir)
if args.developerId != None and args.localId != None:
    cmakeParams.append ('-DAC_MDID_DEV=' + args.developerId)
    cmakeParams.append ('-DAC_MDID_LOC=' + args.localId)
cmakeParams.append (rootPath)

projGenResult = subprocess.call (cmakeParams)
if projGenResult != 0:
    PrintError ('Failed to generate project.')
    sys.exit (1)

# Build

for buildConfig in ['Debug', 'Release']:
    buildResult = subprocess.call ([
        cmake,
        '--build', buildDir,
        '--config', buildConfig
    ])
    if buildResult != 0:
        PrintError ('Failed to build project.')
        sys.exit (1)

sys.exit (0)
