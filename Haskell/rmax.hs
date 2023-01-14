{-
  Author: straykevin
  Description: Recursively deletes the last occurance of the largest item in the list.
  Language: Haskell
  
-}
rmax :: Ord a => [a] -> [a]
rmax [] = []
rmax xs =
  let m = find_max xs
      c = count m xs
   in remove m c xs
  where
    find_max :: Ord t => [t] -> t
    find_max [x] = x
    find_max (x : xs) =
      let m = find_max xs
       in if m > x then m else x


    count :: Ord t => t -> [t] -> Int
    count a [] = 0
    count a (x : xs) =
      if x == a
        then 1 + count a xs
        else count a xs


    remove :: Ord t => t -> Int -> [t] -> [t]
    remove a c [] = []
    remove a c (x : xs)
      | x /= a = x : remove a c xs
      | x == a && c == 1 = xs
      | c < 1 = x : xs
      | otherwise = x : remove a (c - 1) xs
