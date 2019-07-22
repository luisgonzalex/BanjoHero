function show(block) {
  let x = document.getElementById("code" + block);
  if (x.style.display === "none") {
    x.style.display = "block";
    document.getElementById("btn" + block).innerText = "Hide Code";
  } else {
    x.style.display = "none";
    document.getElementById("btn" + block).innerText = "Show Code";
  }
}

for (let i = 1; i < 10; i++) {
  document.getElementById("code" + i).style.display = "none";
}
