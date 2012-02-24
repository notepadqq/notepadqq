#
# Regular cron jobs for the notepadqq package
#
0 4	* * *	root	[ -x /usr/bin/notepadqq_maintenance ] && /usr/bin/notepadqq_maintenance
