--- arch_init.c	2014-04-09 16:59:13.000000000 +0530
+++ ../qemu-master-pre-copy-clone-with-skip/arch_init.c	2014-04-09 11:28:34.000000000 +0530
@@ -60,6 +60,15 @@
     do { } while (0)
 #endif

+//#define CENDHU_DEBUG
+#ifdef CENDHU_DEBUG
+#define DPRINTF_C(fmt, ...) \
+    do { printf("arch_init.c: " fmt, ## __VA_ARGS__); } while (0)
+#else
+#define DPRINTF_C(fmt, ...) \
+    do { } while (0)
+#endif
+
 #ifdef TARGET_SPARC
 int graphic_width = 1024;
 int graphic_height = 768;
@@ -317,8 +326,16 @@
 static RAMBlock *last_sent_block;
 static ram_addr_t last_offset;
 static unsigned long *migration_bitmap;
+static unsigned long *skip_bitmap;
 static uint64_t migration_dirty_pages;
+static uint64_t normal_pages_sent;
+static uint64_t zero_pages_sent;
+static uint64_t pages_skipped;
+static uint64_t total_pages_skipped;
+static uint64_t total_pages_sent;
+static uint64_t pre_copy_round;
 static uint32_t last_version;
+static uint32_t ram_pages;
 static bool ram_bulk_stage;

 /* Update the xbzrle cache to reflect a page that's been sent as all 0.
@@ -404,18 +421,70 @@
     unsigned long size = base + (mr_size >> TARGET_PAGE_BITS);

     unsigned long next;
+    while (true) {
+        if (ram_bulk_stage && nr > base) {
+            next = nr + 1;
+        } else {
+            next = find_next_bit(migration_bitmap, size, nr);
+        }

-    if (ram_bulk_stage && nr > base) {
-        next = nr + 1;
-    } else {
-        next = find_next_bit(migration_bitmap, size, nr);
+        if (next < size) {
+            clear_bit(next, migration_bitmap);
+            migration_dirty_pages--;
+            if (!test_bit(next, skip_bitmap)) {
+                break;
+            } else {
+                pages_skipped++;
+                nr++;
+                continue;
+            }
+        } else {
+            break;
+        }
     }
+    return (next - base) << TARGET_PAGE_BITS;
+}

-    if (next < size) {
-        clear_bit(next, migration_bitmap);
-        migration_dirty_pages--;
+static inline void skip_bitmap_set_dirty(ram_addr_t addr)
+{
+    bool ret;
+    int nr = addr >> TARGET_PAGE_BITS;
+
+    set_bit(nr, skip_bitmap);
+}
+
+static void skip_bitmap_sync_range(ram_addr_t start, ram_addr_t length)
+{
+    ram_addr_t addr;
+    unsigned long page = BIT_WORD(start >> TARGET_PAGE_BITS);
+
+    int k;
+    int nr = BITS_TO_LONGS(length >> TARGET_PAGE_BITS);
+    unsigned long *src = ram_list.dirty_memory[DIRTY_MEMORY_MIGRATION];
+
+    for (k = page; k < page + nr; k++) {
+      if (src[k]) {
+          skip_bitmap[k] |= src[k];
+      }
     }
-    return (next - base) << TARGET_PAGE_BITS;
+}
+
+static void skip_bitmap_sync(void)
+{
+    RAMBlock *block;
+    ram_addr_t addr, offset;
+    offset = last_offset;
+    MigrationState *s = migrate_get_current();
+
+    trace_migration_bitmap_sync_start();
+    address_space_sync_dirty_bitmap(&address_space_memory);
+
+    for (block = last_seen_block; block; block = block->next.tqe_next) {
+        if (block->mr->ram_addr + offset < block->length) {
+            skip_bitmap_sync_range(block->mr->ram_addr, block->length);
+        }
+    }
+    trace_migration_bitmap_sync_end(0);
 }

 static inline bool migration_bitmap_set_dirty(ram_addr_t addr)
@@ -583,6 +652,7 @@
                 if (ret != RAM_SAVE_CONTROL_DELAYED) {
                     if (bytes_sent > 0) {
                         acct_info.norm_pages++;
+                        normal_pages_sent++;
                     } else if (bytes_sent == 0) {
                         acct_info.dup_pages++;
                     }
@@ -593,6 +663,7 @@
                                             RAM_SAVE_FLAG_COMPRESS);
                 qemu_put_byte(f, 0);
                 bytes_sent++;
+                zero_pages_sent++;
                 /* Must let xbzrle know, otherwise a previous (now 0'd) cached
                  * page would be stale
                  */
@@ -624,6 +695,7 @@
                 }
                 bytes_sent += TARGET_PAGE_SIZE;
                 acct_info.norm_pages++;
+                normal_pages_sent++;
             }

             XBZRLE_cache_unlock();
@@ -722,16 +794,20 @@
 }

 #define MAX_WAIT 50 /* ms, half buffered_file limit */
+FILE *migration_log_file;

 static int ram_save_setup(QEMUFile *f, void *opaque)
 {
     RAMBlock *block;
-    int64_t ram_pages = last_ram_offset() >> TARGET_PAGE_BITS;
+    ram_pages = last_ram_offset() >> TARGET_PAGE_BITS;
+    migration_log_file = fopen("/var/log/migration.txt", "a");

     migration_bitmap = bitmap_new(ram_pages);
+    skip_bitmap = bitmap_new(ram_pages);
     bitmap_set(migration_bitmap, 0, ram_pages);
     migration_dirty_pages = ram_pages;
     mig_throttle_on = false;
+    pre_copy_round = 1;
     dirty_rate_high_cnt = 0;

     if (migrate_use_xbzrle()) {
@@ -812,6 +888,11 @@
     while ((ret = qemu_file_rate_limit(f)) == 0) {
         int bytes_sent;

+        if (!(normal_pages_sent % 1024)) {
+            qemu_mutex_lock_iothread();
+            skip_bitmap_sync();
+            qemu_mutex_unlock_iothread();
+        }
         bytes_sent = ram_save_block(f, false);
         /* no more blocks to sent */
         if (bytes_sent == 0) {
@@ -863,6 +944,8 @@

 static int ram_save_complete(QEMUFile *f, void *opaque)
 {
+
+    fclose(migration_log_file);
     qemu_mutex_lock_ramlist();
     migration_bitmap_sync();

@@ -891,19 +974,41 @@
     return 0;
 }

-static uint64_t ram_save_pending(QEMUFile *f, void *opaque, uint64_t max_size)
+static bool round_active = true;
+
+static int64_t ram_save_pending(QEMUFile *f, void *opaque, uint64_t max_size)
 {
     uint64_t remaining_size;

     remaining_size = ram_save_remaining() * TARGET_PAGE_SIZE;

-    if (remaining_size < max_size) {
+    if (!remaining_size) {
+        bitmap_clear(skip_bitmap, 0, ram_pages);
         qemu_mutex_lock_iothread();
         migration_bitmap_sync();
         qemu_mutex_unlock_iothread();
+        round_active = false;
         remaining_size = ram_save_remaining() * TARGET_PAGE_SIZE;
+
+        fprintf(migration_log_file, "%lu normal %lu zero %lu skipped %lu dirted %lu ",
+                pre_copy_round, normal_pages_sent, zero_pages_sent, pages_skipped,
+                migration_dirty_pages);
+        fflush(migration_log_file);
+
+        DPRINTF_C("round %lu normal %lu zero %lu skipped %lu dirted %lu \n",
+                pre_copy_round, normal_pages_sent, zero_pages_sent, pages_skipped,
+                migration_dirty_pages);
+
+        pre_copy_round++;
+        zero_pages_sent = 0;
+        normal_pages_sent = 0;
+        pages_skipped = 0;
+    }
+    if (!round_active) {
+        round_active = true;
+        return remaining_size;
     }
-    return remaining_size;
+    return -1;
 }

 static int load_xbzrle(QEMUFile *f, ram_addr_t addr, void *host)
