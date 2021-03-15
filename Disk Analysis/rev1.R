# load in datasets
normal_ds <- c()
i=1
for(file in c(
  "/media/mark/Backup/fio/lat2/none1.csv",
  "/media/mark/Backup/fio/lat2/none2.csv",
  "/media/mark/Backup/fio/lat2/none3.csv",
  "/media/mark/Backup/fio/lat2/none4.csv",
  "/media/mark/Backup/fio/lat2/none5.csv"
)){
  normal_ds[[i]] <- head(read.csv(file, header = FALSE)$V2, 700)
  i=i+1
}
vibration_ds <- c()
i=1
for(file in c(
  "/media/mark/Backup/fio/lat2/taps_4hz.csv",
  "/media/mark/Backup/fio/lat2/taps_fast.csv",
  "/media/mark/Backup/fio/lat2/taps_knocking.csv",
  "/media/mark/Backup/fio/lat2/taps_knocking_half.csv"
)){
  vibration_ds[[i]] <- head(read.csv(file, header = FALSE)$V2, 700)
  i=i+1
}

# use distance time warping to find overlap
library(proxy);
library(dtw);

# find the best dtw distance on a sliding scale of 100:
# over each window period find the min match, find some alignment for the two series
# use this alignment to compare two smaller dataseries with dtw/euclidean whole
# while the same seed should generate identical series, vibration may
# slow things down
best_overlap <- function(ds1, ds2) {
  window <- 20
  min <- NA
  minI <- NA
  minJ <- NA
  for (i in seq(1, length(ds1)-window, by=window))
  {
    for (j in seq(1, length(ds1)-window, by=window/4))
    {
      alignment<-dtw(ds1[i:(i+window)],ds2[j:(j+window)],keep=TRUE);
      if(is.na(min) | alignment[["distance"]] < min){
        min <- alignment[["distance"]]
        minI <- i
        minJ <- j
      }
    }
  }
  if(minI > minJ){
    offset = minI - minJ
    end = length(ds1) - offset
    alignment<-dtw(ds1[offset:length(ds1)],ds2[1:end],keep=TRUE);
  } else {
    offset = minJ - minI
    end = length(ds1) - offset
    alignment<-dtw(ds1[1:end],ds2[offset:length(ds1)],keep=TRUE);
  }
  plot(alignment,type="threeway")
  alignment[["distance"]]
}

library(combinat)
i=1
list_none = c()
cominations <- combn(seq(1, length(normal_ds)), 2)
for(col in 1:ncol(cominations)) {
  index1 <- cominations[1, col]
  index2 <- cominations[2, col]
  list_none[[i]] <-  best_overlap(normal_ds[[index1]], normal_ds[[index2]])
  i=i+1
}

i=1
list_vibration = c()
cominations <- expand.grid(seq(1, length(normal_ds)), seq(1, length(vibration_ds)))
for(col in seq(1, length(cominations[[1]]))) {
  index1 <- cominations[[1]][col]
  index2 <- cominations[[2]][col]
  list_vibration[[i]] <-best_overlap(normal_ds[[index1]], vibration_ds[[index2]])
  i=i+1
}

stripchart(list("no vibration"= list_none,
                "vibration"= list_vibration),
           main="",
           xlab="Distance",
           ylab="",
           method="jitter",
           col=c("blue","red"),
           pch=16
)
