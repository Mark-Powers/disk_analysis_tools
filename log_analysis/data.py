'''
A bunch of lists with file name groups. Each list is a grouping
of different trail runs on, and each element within a list is the 
type of similar tests. The first element in each list should be a
control test (i.e. with no vibration) so the analysis can use this
as a baseline.
'''

rand_write_barr = [
        "sg_barr-rand-write-none_",
        "sg_barr-rand-write-none2_",
        "sg_barr-rand-write-none3_",
        "sg_barr-rand-write-15Hz-7.6mmps_",
        "sg_barr-rand-write-30Hz-7.6mmps_",
        "sg_barr-rand-write-45Hz-7.6mmps_",
        ]
rand_read_barr = [
        "sg_barr-rand-read-none1_",
        "sg_barr-rand-read-none2_",
        "sg_barr-rand-read-none3_",
        "sg_barr-rand-read-45Hz-7.6mmps_",
        ]
seq_write_barr = [
        "sg_barr-seq-write-none1_",
        "sg_barr-seq-write-45Hz-5.5mmps_",
        ]
seq_write_barr = [
        "sg_barr-seq-write-none1_",
        "sg_barr-seq-write-45Hz-5.5mmps_",
        ]
zero_write_barr = [
        "sg_barr-zero-write-none_",
        "sg_barr-zero-write-45Hz-7.6mmps_",
        ]
rt_vs_no_rt = [
    "sg_barr-rand-write-none_", # RT baseline
    "sg_barr-rand-write-none2_", # RT sample
    "sg_barr-new-none-rand-write_", # no RT here
        ]
sg_barr_max_seek = [
    "sg_brr-rand-write-none-max-seek_",
    "sg_barr-rand-write-none_",
    "sg_barr-rand-write-none2-max-seek_",
        ]

sg_barr_big_jump = [
    "sg_barr-rand-write-none-big_jump_",
    "sg_barr-rand-write-none2-big_jump_",
    "sg_barr-rand-write-none3-big_jump_",
    "sg_barr-rand-write-35Hz-6.0mmps-big_jump_",
    "sg_barr-rand-write-45Hz-8.8mmps-big_jump_"
        ]
'''
sg_barr_big_jump = [
    "sg_barr-rand-write-none_",
    "sg_barr-rand-write-none2_",
    "sg_barr-rand-write-none3_",
    "sg_barr-rand-write-35Hz-6.0mmps-standard_",
    "sg_barr-rand-write-45Hz-6.0mmps-standard_"
        ]
        '''
