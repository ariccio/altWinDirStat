.Language=English,English

# Plugin name
plugin.name = <(NAME)>

# Device list menu
device.list.title = Device list

# Disk menu
disk.menu.pda = pda
disk.menu.smartphone = smartphone

# Create file list
create.list.progress.title = Scanning directories...
create.list.progress.objects = Files: %u; Dirs: %u; Errors: %u
create.list.progress.size = Total size: %S

# Copy files
copy.files.dlg.title = Copy files
copy.files.dlg.path = &Copy files to:
copy.files.dlg.oo = When destination file exists:
copy.files.dlg.oo.skip = &Skip
copy.files.dlg.oo.overwrite = &Overwrite
copy.files.dlg.oo.ask = &Ask
copy.files.dlg.move.files = &Move files
copy.files.dlg.copy.shared = Copy files open for &writing

copy.files.progress.title = Copying files...
move.files.progress.title = Moving files...
move.remote.files.progress.title = Moving files...
copy.files.progress.src = Source:
copy.files.progress.dst = Destination:
copy.files.progress.size = Processed %7S from %7S [%3u%%] at %9S
copy.files.progress.file.size = Processed %7S from %7S [%3u%%] at %9S
copy.files.progress.time = Elapsed: %8S  Left: %8S  Total: %8S
move.remote.files.progress.time = Elapsed: %8S
copy.files.progress.stats = Files(Ov/Skip): %u(%u/%u); Dirs: %u; Errors: %u
move.remote.files.progress.stats = Files: %u; Dirs: %u

overwrite.dlg.title = Overwrite file
overwrite.dlg.directory.title = Overwrite directory
overwrite.dlg.question = Destination file already exists. Overwrite?
overwrite.dlg.directory.question = Destination directory already exists. Overwrite?
overwrite.dlg.source = Source:
overwrite.dlg.destination = Destination:
overwrite.dlg.newer = (newer)
overwrite.dlg.yes = &Yes
overwrite.dlg.yes.all = Yes to &All
overwrite.dlg.no = &No
overwrite.dlg.no.all = No to A&ll

copy.files.stats.title = Copy finished
copy.files.stats.errors = Errors encountered: %u
copy.files.stats.files = Files copied/overwritten/skipped: %u/%u/%u
copy.files.stats.dirs = Directories processed: %u

# Delete files
delete.files.dlg.title = Delete files
delete.files.dlg.label = Delete selected files

delete.files.progress.title = Delete in progress...
delete.files.progress.time = Elapsed: %8S  Left: %8S  Total: %8S
delete.files.progress.stats = Files: %u; Dirs: %u; Errors: %u

delete.files.stats.title = Delete finished
delete.files.stats.errors = Errors encountered: %u
delete.files.stats.files = Files deleted: %u
delete.files.stats.dirs = Directories deleted: %u

# Create directory
create.dir.dlg.title = Create directory
create.dir.dlg.path = &Create directory:

# Plugin options
options.dlg.title = Options
options.dlg.disk.menu = Add to &disk menu
options.dlg.plugin.menu = Add to &plugin menu
options.dlg.disk.menu.number = Disk &menu number:
options.dlg.hide.copy.dlg = D&o not show own copy dialog
options.dlg.save.last.dir = &Save last directory
options.dlg.use.file.filters = &Use ActiveSync file filters
options.dlg.access.method = Device access method (restart needed):
options.dlg.rapi = &RAPI
options.dlg.rapi2 = RAPI&2
options.dlg.hide.rom.files = &Hide ROM files
options.dlg.buffer.size = &Buffer size:
options.dlg.auto.buf.size = &Automatic
options.dlg.prefix = P&lugin prefix (restart needed):
options.dlg.save.def.values = Sa&ve options from dialogs
options.dlg.def.option.values.title = Default Option Values
options.dlg.def.option.values = Add&itional
options.dlg.hot.keys.title = Hot Keys
options.dlg.hot.keys = Hot &Keys
options.dlg.hot.key.execute = &Run file on device:
options.dlg.hot.key.attr = Change file &attributes:
options.dlg.hot.key.hide.rom.files = &Hide/show ROM files:
options.dlg.exit.on.dot.dot = &Exit plugin on ".." from root
options.dlg.show.free.space = Show &free space in panel title

# Common options
options.ignore.errors = &Ignore errors
options.show.stats = Show statistics after file operation:
options.show.stats.always = A&lways
options.show.stats.never = &Never
options.show.stats.if.error = If &error

# File attributes
attr.dlg.title = File Attributes
attr.dlg.read.only = &Read Only
attr.dlg.sparse = S&parse
attr.dlg.archive = &Archive
attr.dlg.temporary = &Temporary
attr.dlg.hidden = &Hidden
attr.dlg.reparse.point = Reparse P&oint
attr.dlg.system = &System
attr.dlg.in.rom = &In ROM
attr.dlg.compressed = &Compressed
attr.dlg.rom.module = RO&M Module
attr.dlg.encrypted = &Encrypted

# System info
sysinfo.device.info = Device Information
sysinfo.os.version = OS Version:
sysinfo.program.memory = Program Memory
sysinfo.storage.memory = Storage Memory
sysinfo.storage.card = Storage Card Space
sysinfo.total = Total:
sysinfo.free = Free:
sysinfo.power = Power Status
sysinfo.power.type = Power Type:
sysinfo.power.online = AC power
sysinfo.power.offline = Battery
sysinfo.power.backup = Backup battery
sysinfo.power.battery.level = Battery Level:
sysinfo.power.backup.battery.level = Backup Battery Level:

# Run dialog
run.dlg.title = Run
run.dlg.cmd.line = &Command line:

# File filters
filter.dlg.title = File Filters
filter.dlg.select.files = Select file types to convert:
filter.dlg.to = to

# Generic buttons
button.ok = OK
button.cancel = Cancel
button.retry = &Retry
button.ignore = &Ignore
button.ignore.all = Ignore &All

# Size and speed suffixes
suffix.size.b = B
suffix.size.kb = KB
suffix.size.mb = MB
suffix.size.gb = GB
suffix.size.tb = TB
suffix.speed.b = B/s
suffix.speed.kb = KB/s
suffix.speed.mb = MB/s
suffix.speed.gb = GB/s
suffix.speed.tb = TB/s

# Error log
log.title = Error Log
log.scan.dir = Read directory
log.create.dir = Create directory
log.copy.file = Copy file
log.delete.file = Delete file
log.delete.dir = Delete directory
log.show = &Show error log

# Information & progress messages
progress.prepare = Preparing...
progress.filter = File filter is processing file...
progress.interrupt = Do you want to cancel current operation?

# Error messages
err.no.activesync = ActiveSync is not installed or outdated
err.no.device = No device connected
err.plugin.init = Cannot create plugin instance
err.self.copy = Cannot copy file to itself
err.device.disconnected = Device was disconnected
err.invalid.cmd.line = Command line specified is invalid
err.fatal = Unexpected exception occured. This is probably a bug.
err.fatal.dump = Minidump successfully saved in plugin directory.
err.fatal.no.dump = Install dbghelp.dll to be able to save minidump.
err.com.model = COM is already initialized by another pligin for apartment-threaded object concurrency. This is known to cause problems with WMDC on Windows Vista. Please switch to RAPI mode in plugin options.
err.not.activesync.user = ActiveSync is installed but not initialized for current user.
err.same.name.exists = Different object with the same name already exists.
err.invalid.hot.key = '%s' is not a valid hot key.
err.invalid.prefix = Plugin prefix is not specified.
