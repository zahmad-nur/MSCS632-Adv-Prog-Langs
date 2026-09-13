//Section 2 JavaScript Combine Program

function combine(a, b) {
  return a + b;
}

console.log(combine(5, 3));      // 8
console.log(combine("5", "3"));  // "53"
console.log(combine(5, "3"));    // "53"  (number coerced to string)