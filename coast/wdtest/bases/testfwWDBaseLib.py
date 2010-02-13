import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
	packagename : {
		'linkDependencies' : ['testfwFoundation', 'CoastWDBase'],
                     'includeSubdir'    : '',
                     'sourceFiles'      : SConsider.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'lazylinking'      : True,
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                 'includes'     : SConsider.listFiles(['*.h']),
                    }
                 }
                }

SConsider.createTargets(packagename, buildSettings)
