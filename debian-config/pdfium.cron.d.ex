#
# Regular cron jobs for the pdfium package
#
0 4	* * *	root	[ -x /usr/bin/pdfium_maintenance ] && /usr/bin/pdfium_maintenance
