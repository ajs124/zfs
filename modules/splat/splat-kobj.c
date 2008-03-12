#include "splat-internal.h"

#define SPLAT_SUBSYSTEM_KOBJ		0x0a00
#define SPLAT_KOBJ_NAME			"kobj"
#define SPLAT_KOBJ_DESC			"Kernel File Tests"

#define SPLAT_KOBJ_TEST1_ID		0x0a01
#define SPLAT_KOBJ_TEST1_NAME		"open"
#define SPLAT_KOBJ_TEST1_DESC		"File Open/Close Test"

#define SPLAT_KOBJ_TEST2_ID		0x0a02
#define SPLAT_KOBJ_TEST2_NAME		"size/read"
#define SPLAT_KOBJ_TEST2_DESC		"File Size/Read Test"

#define SPLAT_KOBJ_TEST_FILE		"/etc/fstab"

static int
splat_kobj_test1(struct file *file, void *arg)
{
	struct _buf *f;

	f = kobj_open_file(SPLAT_KOBJ_TEST_FILE);
	if (f == (struct _buf *)-1) {
		splat_vprint(file, SPLAT_KOBJ_TEST1_NAME, "Failed to open "
			     "test file: %s\n", SPLAT_KOBJ_TEST_FILE);
		return -ENOENT;
	}

	kobj_close_file(f);
	splat_vprint(file, SPLAT_KOBJ_TEST1_NAME, "Successfully opened and "
		     "closed test file: %s\n", SPLAT_KOBJ_TEST_FILE);

        return 0;
} /* splat_kobj_test1() */

static int
splat_kobj_test2(struct file *file, void *arg)
{
	struct _buf *f;
	char *buf;
	uint64_t size;
	int rc;

	f = kobj_open_file(SPLAT_KOBJ_TEST_FILE);
	if (f == (struct _buf *)-1) {
		splat_vprint(file, SPLAT_KOBJ_TEST2_NAME, "Failed to open "
			     "test file: %s\n", SPLAT_KOBJ_TEST_FILE);
		return -ENOENT;
	}

	rc = kobj_get_filesize(f, &size);
	if (rc) {
		splat_vprint(file, SPLAT_KOBJ_TEST2_NAME, "Failed stat of "
			     "test file: %s (%d)\n", SPLAT_KOBJ_TEST_FILE, rc);
		goto out;
	}

	buf = kmalloc(size, GFP_KERNEL);
	if (!buf) {
		rc = -ENOMEM;
		splat_vprint(file, SPLAT_KOBJ_TEST2_NAME, "Failed to alloc "
			     "%lld bytes for tmp buffer (%d)\n", size, rc);
		goto out;
	}

	rc = kobj_read_file(f, buf, size, 0);
	if (rc < 0) {
		splat_vprint(file, SPLAT_KOBJ_TEST2_NAME, "Failed read of "
			     "test file: %s (%d)\n", SPLAT_KOBJ_TEST_FILE, rc);
		goto out2;
	}

	/* Validate we read as many bytes as expected based on the stat.  This
	 * isn't a perfect test since we didn't create the file however it is
	 * pretty unlikely there are garbage characters in your /etc/fstab */
	if (size != (uint64_t)strlen(buf)) {
		rc = EFBIG;
		splat_vprint(file, SPLAT_KOBJ_TEST2_NAME, "Stat'ed size "
			     "(%lld) does not match number of bytes read "
			     "(%lld)\n", size, (uint64_t)strlen(buf));
		goto out2;
	}

	rc = 0;
	splat_vprint(file, SPLAT_KOBJ_TEST2_NAME, "\n%s\n", buf);
	splat_vprint(file, SPLAT_KOBJ_TEST2_NAME, "Successfully stat'ed "
		     "and read expected number of bytes (%lld) from test "
		     "file: %s\n", size, SPLAT_KOBJ_TEST_FILE);
out2:
	kfree(buf);
out:
	kobj_close_file(f);

        return rc;
} /* splat_kobj_test2() */

splat_subsystem_t *
splat_kobj_init(void)
{
        splat_subsystem_t *sub;

        sub = kmalloc(sizeof(*sub), GFP_KERNEL);
        if (sub == NULL)
                return NULL;

        memset(sub, 0, sizeof(*sub));
        strncpy(sub->desc.name, SPLAT_KOBJ_NAME, SPLAT_NAME_SIZE);
	strncpy(sub->desc.desc, SPLAT_KOBJ_DESC, SPLAT_DESC_SIZE);
        INIT_LIST_HEAD(&sub->subsystem_list);
	INIT_LIST_HEAD(&sub->test_list);
        spin_lock_init(&sub->test_lock);
        sub->desc.id = SPLAT_SUBSYSTEM_KOBJ;

        SPLAT_TEST_INIT(sub, SPLAT_KOBJ_TEST1_NAME, SPLAT_KOBJ_TEST1_DESC,
	              SPLAT_KOBJ_TEST1_ID, splat_kobj_test1);
        SPLAT_TEST_INIT(sub, SPLAT_KOBJ_TEST2_NAME, SPLAT_KOBJ_TEST2_DESC,
	              SPLAT_KOBJ_TEST2_ID, splat_kobj_test2);

        return sub;
} /* splat_kobj_init() */

void
splat_kobj_fini(splat_subsystem_t *sub)
{
        ASSERT(sub);

        SPLAT_TEST_FINI(sub, SPLAT_KOBJ_TEST2_ID);
        SPLAT_TEST_FINI(sub, SPLAT_KOBJ_TEST1_ID);

        kfree(sub);
} /* splat_kobj_fini() */

int
splat_kobj_id(void)
{
        return SPLAT_SUBSYSTEM_KOBJ;
} /* splat_kobj_id() */
