# load in datasets
normal1 = head(read.csv("/media/mark/Backup/fio/lat/lat_normal1.csv"), 700)
normal2 = head(read.csv("/media/mark/Backup/fio/lat/lat_normal2.csv"), 700)
normal3 = head(read.csv("/media/mark/Backup/fio/lat/lat_normal3.csv"), 700)
normal4 = head(read.csv("/media/mark/Backup/fio/lat/lat_normal4.csv"), 700)
normal5 = head(read.csv("/media/mark/Backup/fio/lat/lat_normal5.csv"), 700)
taps1 = head(read.csv("/media/mark/Backup/fio/lat/lat_taps1.csv"), 700)
taps2 = head(read.csv("/media/mark/Backup/fio/lat/lat_taps2.csv"), 700)
taps_half = head(read.csv("/media/mark/Backup/fio/lat/lat_taps_half.csv"), 700)

# summarize datasets
summary(normal1$lat)
summary(normal2$lat)
summary(normal3$lat)
summary(normal4$lat)
summary(normal5$lat)
summary(taps1$lat)
summary(taps2$lat)
summary(taps_half$lat)

# euclidean distance of lat column
lat_dist <- function(list1, list2) {
  sqrt(sum((list1$lat - list2$lat)^2))
}
# plot two types of expected distance next to each other
stripchart(
  list("normal"=c(lat_dist(normal1, normal2), lat_dist(normal1, normal3), lat_dist(normal2, normal3)),
       "taps"= c(lat_dist(normal1, taps1), lat_dist(normal1, taps2), #lat_dist(normal1, taps_half),
                 lat_dist(normal2, taps1), lat_dist(normal2, taps2), #lat_dist(normal2, taps_half),
                 lat_dist(normal3, taps1), lat_dist(normal3, taps2)#,lat_dist(normal3, taps_half),
                 )),
           main="",
           xlab="Distance",
           ylab="",
           method="jitter",
           col=c("blue","red"),
           pch=16
)

# sort and plot lat values to see distribution
plot(seq(1, 700, by=1), sort(normal1$lat))
plot(seq(1, 700, by=1), sort(normal2$lat))
plot(seq(1, 700, by=1), sort(taps1$lat))
plot(seq(1, 700, by=1), sort(taps2$lat))

# use distance time warping to find overlap
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
list_normal <- c(best_overlap(normal1$lat, normal2$lat),
                 best_overlap(normal1$lat, normal3$lat),
                 #best_overlap(normal1$lat, normal4$lat),
                 #best_overlap(normal1$lat, normal5$lat),
                 best_overlap(normal2$lat, normal3$lat)#,
                 #best_overlap(normal2$lat, normal4$lat),
                 #best_overlap(normal2$lat, normal5$lat),
                 #best_overlap(normal3$lat, normal4$lat),
                 #best_overlap(normal3$lat, normal5$lat),
                 #best_overlap(normal4$lat, normal5$lat)
                 )
list_taps1 <- c(best_overlap(normal1$lat, taps1$lat),
                   best_overlap(normal2$lat, taps1$lat),
                   best_overlap(normal3$lat, taps1$lat))
list_taps2 <- c(best_overlap(normal1$lat, taps2$lat),
                   best_overlap(normal2$lat, taps2$lat),
                   best_overlap(normal3$lat, taps2$lat))
stripchart(list("no vibration"= list_normal,
       "vibration 1"= list_taps1,
       "vibration 2"= list_taps2),
  main="",
  xlab="Distance",
  ylab="",
  method="jitter",
  col=c("blue","red"),
  pch=16
)
