module Main where

log2 :: Integer -> Integer
log2 0 = 0
log2 n = ceiling . logBase 2.0 . fromIntegral $ n

permutationCost :: Integer -> Integer
permutationCost n = n * log2 n - n + 1

-- After access i, what size permutation is needed?
permutations :: [Integer]
permutations = map ((2*) . maxDivPow2) [1..]
  where
    maxDivPow2 i = go i 1
    go i pow =
      if i `mod` pow == 0
      then max pow (go i (2*pow))
      else 0

scan :: Integer -> Integer -> Integer
scan w n = w*(n-1)*2

stack :: Integer -> Integer -> Integer
stack w n = 4*w * (levels 1 n)
  where
    -- how many levels of stack are needed to hold the n elements?
    levels l n =
      if 3 * (2^l - 1) >= n
      then l
      else levels (l+1) n

lazyPermute :: Integer -> Integer -> Integer
lazyPermute w n = loop n
  where
    loop 1 = (w+1)*log2 n
    loop i = 2*stack (w + log2 i - 1) (i`div`2) + loop (i `div` 2) + w - 1


hide :: Integer -> Integer -> Integer
hide w n = (w+1) * log2 n `div` 2

access :: Integer -> Integer -> Integer
access w n = 2 * lazyPermute (w+1) (2*n) + (pcost `div` n) + hide w n
  where
    pcost = w * sum (map permutationCost (n : take (fromIntegral n) permutations))


recursion :: Integer -> Integer
recursion bits =
  let w = 2 * log2 bits
      n = (bits + w - 1) `div` w
      acc = access w n
  in if (2*bits) <= acc
      then 2*bits
      -- extra w is to linear scan the two elements accessed.
      else acc + min (2*bits) (w + recursion (n*(log2 n + 1)))


cost :: Integer -> Integer -> Integer
cost w n = access w n + recursion (n*(log2 n + 1))


comp :: Integer -> Integer -> Double
comp w n = fromIntegral (scan w n) / fromIntegral (cost w n)


main :: IO ()
main = go bot
  where
    go n =
      if n > top
      then pure ()
      else do
        putStr "("
        putStr (show n)
        putStr ","
        -- putStr (show (cost w n * 16))
        -- putStr (show (scan w n * 16))
        putStr (show ((2^15) * log2 n * log2 n))
        putStrLn ")"


        -- putStr ","
        -- putStr (show (cost w n))
        -- putStrLn ""
        go (n*2)


    w = 128
    bot = 2
    top = 2^20
