@echo off

cutechess-cli -engine name=Base cmd=./crystall.exe ^
              -engine name=New  cmd=./crystall_new.exe ^
              -each tc=10+0.1 proto=uci ^
              -games 1000 ^
              -sprt alpha=0.05 beta=0.05 elo0=0 elo1=10 ^
              -concurrency 6 ^
              -repeat ^
              -pgnout games.pgn ^
              -openings file=book.epd format=epd order=random