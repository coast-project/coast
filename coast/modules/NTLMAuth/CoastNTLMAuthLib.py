import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
                 packagename : {
                     'linkDependencies' : ['CoastSecurity', 'CoastWDBase', 'openssl'],
                     'sourceFiles'      : SConsider.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                 'includes'     : SConsider.listFiles(['*.h']),
                                 'includeSubdir'    : '',
                    }
                 }
                }

SConsider.createTargets(packagename, buildSettings)
