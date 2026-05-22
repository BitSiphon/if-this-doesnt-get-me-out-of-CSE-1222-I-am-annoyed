package main

import (
	"fmt"
	"net/http"
)

func HelloBroadcast(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "Hello from Go.")
}

func main() {
	http.HandleFunc("/hello", HelloBroadcast)

	http.ListenAndServe(":8080", nil)
}
