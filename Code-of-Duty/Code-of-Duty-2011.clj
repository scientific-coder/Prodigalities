":";exec java -cp "/usr/share/java/clojure.jar" clojure.main $0 $*
(ns code-of-duty-2011) ;; (C) Bernard Hugueney, GPL v3 or later.
(defn- parse-vect 
  "parse sequences of [size elts] into a vector of vectors"
  [v]
  (loop [to-parse v
         res []]
    (if (< (count  to-parse) 2)
      res
      (let [[nb & more] to-parse
            [current-v todo] (split-at nb more)]
        (recur todo (conj res current-v))))))
(defn- one-step
  "perform one step of the process on the elts of v with given mean"
  [mean v]
  ;; taking the rest because we prefix the sequence with a
  ;; dummy value to handle first elt of v as a middle of 3 elts
  (next (loop [delta 0
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
(defn- process
  "test if processing is possible and do as many steps as necessary"
  [v]
  (let [mean (/ (apply + v) (count v))]
    (if (integer? mean)
      (loop [current-v v
             res []]
        (let [next-res (conj res current-v)]
          (if (apply = current-v)
            next-res
            (recur (one-step mean current-v) next-res))))
      nil)))
(defn- print-res
  "print result for one processed vector to output"
  [output res]
  (binding [*out* output]
    (do
      (if res
        (do (println (dec (count res)))
            (doseq [[idx x] (map vector (iterate inc 0) res)]
              (println idx " : (" (apply str (interpose ", " x)) ")")))
        (print -1)))
    (println)))
(defn -main
  [& args]
  (with-open [output (java.io.FileWriter. "output.txt")]
    (let [process-and-print (comp (partial print-res output)
                                  process)]
      (dorun (->> "input.txt" slurp (re-seq #"\d+") (map #(Integer/parseInt %))
                  parse-vect 
                  (map process-and-print))))))

(defn command-line? []                               
  (.isAbsolute (java.io.File. *file*)))

(if (command-line?) (-main))
