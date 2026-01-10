function login() {
    const user = document.getElementById("username").value;
    const pass = document.getElementById("password").value;
    const errorEl = document.getElementById("error");

    // Clear previous error
    errorEl.innerText = "";

    if (!user || !pass) {
        errorEl.innerText = "Please enter both username and password";
        return;
    }

    // Simple hardcoded auth for UI demo
    if (user === "admin" && pass === "admin123") {
        // Add loading effect
        const btn = document.querySelector('button');
        btn.innerHTML = "Authenticating...";
        btn.style.opacity = "0.7";

        setTimeout(() => {
            window.location.href = "dashboard.html";
        }, 800);
    } else {
        // Shake animation effect could be added here
        errorEl.innerText = "Invalid credentials. Try admin / admin123";
    }
}
