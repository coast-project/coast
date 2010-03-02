from __future__ import with_statement
import os, subprocess, re, pdb
import SConsider
from stat import *
from SomeUtils import multiple_replace

user1 = {'user': 'oratest', 'pw': 'oratest'}
user2 = {'user': 'oratest2', 'pw': 'oratest2'}
db = {'server': 'sifs-coast1.hsr.ch', 'port': 1521, 'sid': 'orcl'}

packagename = SConsider.getPackageName(__name__)

connectstring = "{user}/{pw}@{server}:{port}/{sid}"
connectstring1 = connectstring.format(**dict(db, **user1))
connectstring2 = connectstring.format(**dict(db, **user2))

def runSQL(connectstring, filename, logpath, filter=None):
    targets = SConsider.programLookup.getPackageTarget('oracle', 'sqlplus')
    target = targets['target']
    if not target or not os.path.isfile(filename):
        return False
    args = [target.abspath, connectstring]
    res = 0
    
    with open(filename) as file:
        sqlplus = subprocess.Popen(args, stdin=subprocess.PIPE,
                                     stderr=subprocess.PIPE,
                                     stdout=subprocess.PIPE)
        content = file.read()
        if callable(filter):
            content = filter(content)
        sqlplus_out, sqlplus_err = sqlplus.communicate(content)

        if not os.path.isdir(logpath):
            os.makedirs(logpath)
        logfilebasename = target.name + '.' + os.path.basename(filename)
        if sqlplus_err:
            with open(os.path.join(logpath, logfilebasename + '.stderr'), 'w') as errfile:
                errfile.write(sqlplus_err)
        if sqlplus_out:
            with open(os.path.join(logpath, logfilebasename + '.stdout'), 'w') as outfile:
                outfile.write(sqlplus_out)

    return res

def setUp(target, source, env):
    if not os.environ.has_key('NLS_LANG'):
        os.environ['NLS_LANG'] = '.WE8ISO8859P1'
    logpath = env['BASEOUTDIR'].Dir(os.path.join('tests', packagename, env['LOGDIR'], env['VARIANTDIR']))
    path = env['BASEOUTDIR'].Dir(os.path.join('tests', packagename)).Dir('config')
    runSQL(connectstring1, path.File('oratest_dropschema.sql').abspath, logpath.abspath)
    runSQL(connectstring1, path.File('oratest_schema.sql').abspath, logpath.abspath)
    runSQL(connectstring1, path.File('oratest_grant.sql').abspath, logpath.abspath,
           filter=lambda string: multiple_replace([
                                                   ("##user1##", user1['user']),
                                                   ("##user2##", user2['user'])
                                                   ], string) )
    runSQL(connectstring2, path.File('oratest_synonym.sql').abspath, logpath.abspath,
           filter=lambda string: multiple_replace([
                                                   ("##user1##", user1['user']),
                                                   ("##user2##", user2['user'])
                                                   ], string) )

def tearDown(target, source, env):
    logpath = env['BASEOUTDIR'].Dir(os.path.join('tests', packagename, env['LOGDIR'], env['VARIANTDIR']))
    path = env['BASEOUTDIR'].Dir(os.path.join('tests', packagename)).Dir('config')
    runSQL(connectstring1, path.File('oratest_dropschema.sql').abspath, logpath.abspath)

buildSettings = {
    packagename : {
        'targetType'       : 'ProgramTest',
        'linkDependencies' : ['CoastOracle', 'testfwWDBase'],
        'requires'         : ['CoastAppLog', 'CoastRenderers', 'oracle.sqlplus'],
        'sourceFiles'      : SConsider.listFiles(['*.cpp']),
        'copyFiles'        : [(SConsider.findFiles(['.'],['.any','.txt','.tst','.sql']), S_IRUSR|S_IRGRP|S_IROTH)],
        'runConfig'        : {
                              'setUp': setUp,
                              'tearDown': tearDown,
                             },
    },
}

SConsider.createTargets(packagename, buildSettings)
