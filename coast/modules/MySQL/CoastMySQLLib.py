import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
                 'CoastMySQL' : {
                     'includeSubdir'    : '',
                     'linkDependencies' : ['mysql', 'CoastDataAccess'],
                     'sourceFiles'      : SConsider.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                 'includes'     : SConsider.listFiles(['*.h']),
#                                'appendUnique' : { 'CPPDEFINES' : 'fooX' },
                    }
                 }
                }

SConsider.createTargets(packagename, buildSettings)
