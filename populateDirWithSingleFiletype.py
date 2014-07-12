import os
import rstr
import sys

NUMBER_OF_FILES_TO_GENERATE = 10000000

def create_in_dir(num, path_to_dir, aRstr):
    if num < 0:
        return
    print("create_in_dir(%i, %s)" % (num, path_to_dir))
    files_in_dir = os.listdir(path_to_dir)
    name = "%s.fart" % (aRstr.rstr(rstr.letters()))
    recurse_dir = []
    errrs = 0
    for itera in range(num):
        if itera % 1000 == 0:
            print("%i of %i " % (itera, num))
        try:
            if name not in files_in_dir:
                with open(name, 'x') as f:
                    #print('\tcreated file: %s' % (name))
                    for _ in range(100):
                        f.write(aRstr.rstr(rstr.letters()))
                        f.write('\n')
                    files_in_dir.append(name)
            else:
                errrs += 1
            name = "%s.fart" % (aRstr.rstr(rstr.letters()))
        except FileExistsError:
            files_in_dir.append(name)
            errrs += 1
            name = "%s.fart" % (aRstr.rstr(rstr.letters()))
        except FileNotFoundError:
            print("File wasn't found?!? (FileNotFoundError). Filename: `%s`" % name)
            errrs += 1
            name = "%s.fart" % (aRstr.rstr(rstr.letters()))
    print("%i errors occurred while processing....\n\n\n\n\n\n\n" % errrs)
def _profile(continuation):
    prof_file = 'populateDir.prof'
    try:
        import cProfile
        import pstats
        print('Profiling using cProfile')
        cProfile.runctx('continuation()', globals(), locals(), prof_file)
        stats = pstats.Stats(prof_file)
    except ImportError:
        import hotshot
        import hotshot.stats
        prof = hotshot.Profile(prof_file, lineevents=1)
        print('Profiling using hotshot')
        prof.runcall(continuation)
        prof.close()
        stats = hotshot.stats.load(prof_file)
    stats.strip_dirs()
    #for a in ['calls', 'cumtime', 'cumulative', 'ncalls', 'time', 'tottime']:
    for a in ['cumtime', 'time', 'ncalls']:
        print("------------------------------------------------------------------------------------------------------------------------------")
        try:
            stats.sort_stats(a)
            stats.print_stats(150)
            stats.print_callees(150)
            stats.print_callers(150)
        except KeyError:
            pass
    os.remove(prof_file)
                
                
                
def main():
    if sys.version_info.major < 3:
        sys.exit("Upgrade to a newer version of Python, please.")
    print("this will populate THE CURRENT (%s) directory with %i files! DO NOT run in same directory as this script!" % (os.getcwd(), NUMBER_OF_FILES_TO_GENERATE))
    i = input("Are you sure you want to continue?(N, NO to abort)")
    if i in ["N", "NO", "n", "no"]:
        sys.exit("aborted")
    if "populateDir.py" in os.listdir():
        i = input("A script with the same name is in this directory. Are you REALLY sure you want to continue?(N, NO to abort)")
        if i in ["N", "NO", "n", "no"]:
            sys.exit("aborted")
    
    aRstr = rstr.Rstr()
    try:
        def safe_main():
            try:
                create_in_dir(NUMBER_OF_FILES_TO_GENERATE, os.getcwd(), aRstr)
            except:
                pass
        _profile(safe_main)
    except:
        pass
    #create_in_dir(NUMBER_OF_FILES_TO_GENERATE, os.getcwd(), aRstr)
    print("Current number of files in dir: ", len(os.listdir()))
if __name__ == "__main__":
    main()
