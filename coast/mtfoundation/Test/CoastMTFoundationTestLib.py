import os
import SConsider
from stat import *

packagename = SConsider.getPackageName( __name__ )

def setUp( target, source, env ):
    None

def tearDown( target, source, env ):
    # delete generated files
    None

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastMTFoundation', 'testfwFoundation'],
        'sourceFiles'      : SConsider.listFiles( ['*.cpp'] ),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [( SConsider.findFiles( ['config'], ['.any'] ), S_IRUSR | S_IRGRP | S_IROTH )],
        'runConfig'        : {
            'setUp'        : setUp,
            'tearDown'     : tearDown,
        },
    }
}

SConsider.createTargets( packagename, buildSettings )
