;; (C) Bernard Hugueney, GPL v3 or later.
(letfn [(;; parse sequences of [size elts] into a vector of vectors
         parse-vect [v]
         (loop [to-parse v
                res []]
           (if (< (count  to-parse) 2)
             res
             (let [[nb & more] to-parse]
               (recur (drop nb more ) (conj res (take nb more)))))))
        (;; perform one step of the process on the elts of v with given mean
         one-step [mean v]
         ;; taking the rest because we prefix the sequence with a
         ;; dummy value to handle first elt of v as a middle of 3 elts
         (rest (loop [delta 0
                      to-process (into [nil] v)
                      res []]
                 (case (count to-process)
                       1 (into res to-process)
                       2 (let [[a0 a1] to-process]
                           (into res (if (> a1 mean)
                                       [(inc a0) (dec a1)]
                                       [a0 a1])))
                       (let [[a0 a1 a2 & r] to-process
                             [b0 b1 b2] (if (< delta 0)
                                          [(inc a0) (dec a1) a2]
                                          (if (> a1 mean)
                                            [a0 (dec a1) (inc a2)]
                                            [a0 a1 a2]))]
                         (recur (- (+ delta b1) mean) (into [b1 b2] r) (conj res b0)))))))
        (;; test if processing is possible and do as many steps as necessary
         process [v]
         (let [mean (/ (apply + v) (count v))]
           (if (integer? mean)
             (loop [current-v v
                    res []]
               (let [next-res (conj res current-v)]
                 (if (apply = current-v)
                   next-res
                   (recur (one-step mean current-v) next-res))))
             nil)))
        (;; convert one result (nil if invalid data) to a string of the requested format
         res-str [res]
         (if res
           (apply str (into [(dec (count res)) "\n"]
                            (for [[idx x] (map vector (iterate inc 0) res)]
                              (str idx " : (" (apply str (interpose ", " x)) ")\n"))))
           "-1\n"))]
  (->> "input.txt" slurp (re-seq #"\d+") (map #(Integer/parseInt %)) parse-vect 
       (pmap #(res-str (process %))) (interpose "\n") (apply str) (spit "my_output.txt"))
