#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");

static int my_date_show(struct seq_file *m, void *v) {
	//timeval is time in seconds (.tv_sec)
	struct timeval sec_time;
	//tm is human-readable time
  	struct tm time_date;
	
	//get current time (xtime)
	do_gettimeofday(&sec_time);

	//convert to human-readable time, no offset (GMT)
	time_to_tm(sec_time.tv_sec, 0, &time_date);

	//Print to proc as: 2014-10-02 15:52:36
	seq_printf(m, "%04ld-%02d-%02d %02d:%02d:%02d\n", time_date.tm_year+1900, time_date.tm_mon+1, time_date.tm_mday, time_date.tm_hour, time_date.tm_min, time_date.tm_sec);
	return 0;
}

static int my_date_open(struct inode *inode, struct  file *file) {
  	return single_open(file, my_date_show, NULL);
}

static const struct file_operations my_date_fops = {
  	.owner = THIS_MODULE,
  	.open = my_date_open,
  	.read = seq_read,
  	.release = single_release,
};

static int __init my_date_init(void) {
  	proc_create("current_date", 0, NULL, &my_date_fops);
  	return 0;
}

static void __exit my_date_exit(void) {
  	remove_proc_entry("current_date", NULL);
}

module_init(my_date_init);
module_exit(my_date_exit);
