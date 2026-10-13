import { useState } from "react";
import { Navigate, useNavigate } from "react-router-dom";
import { useAuth } from "../context/AuthContext";

export default function Login() {
  const { user, login } = useAuth();
  const navigate = useNavigate();

  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [showPassword, setShowPassword] = useState(false);
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);

  if (user) {
    return <Navigate to="/dashboard" replace />;
  }

  const submit = async (e) => {
    e.preventDefault();

    setError("");

    const trimmedEmail = email.trim();

    if (!trimmedEmail || !password) {
      setError("Please enter your email and password.");
      return;
    }

    if (!trimmedEmail.includes("@")) {
      setError("Please enter a valid email address.");
      return;
    }

    try {
      setLoading(true);

      await login(trimmedEmail, password);

      navigate("/dashboard");
    } catch (err) {
      setError(err.message || "Login failed. Please check your credentials.");
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="auth-page">
      <form className="auth-card stack" onSubmit={submit}>
        <h1>📚 Library Login</h1>

        <p className="muted">
          Sign in to access your library portal.
        </p>

        {error && (
          <div className="alert" role="alert">
            {error}
          </div>
        )}

        {/* Email */}
        <div>
          <label htmlFor="email">Email Address</label>

          <input
            id="email"
            className="input"
            type="email"
            placeholder="Enter your email"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
            autoComplete="email"
            required
          />
        </div>

        {/* Password */}
        <div>
          <label htmlFor="password">Password</label>

          <div style={{ position: "relative" }}>
            <input
              id="password"
              className="input"
              type={showPassword ? "text" : "password"}
              placeholder="Enter your password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              autoComplete="current-password"
              required
              style={{ paddingRight: "80px" }}
            />

            <button
              type="button"
              onClick={() => setShowPassword(!showPassword)}
              style={{
                position: "absolute",
                right: "10px",
                top: "50%",
                transform: "translateY(-50%)",
                background: "transparent",
                border: "none",
                cursor: "pointer",
                fontSize: "13px",
                fontWeight: "600",
              }}
            >
              {showPassword ? "Hide" : "Show"}
            </button>
          </div>
        </div>

        {/* Login button */}
        <button
          className="btn btn-primary"
          type="submit"
          disabled={loading}
        >
          {loading ? "Signing in..." : "Login"}
        </button>

        {/* Register */}
        <div
          style={{
            textAlign: "center",
            marginTop: "8px",
            fontSize: "14px",
          }}
        >
          <span className="muted">Don't have an account? </span>

          <button
            type="button"
            onClick={() => navigate("/register")}
            style={{
              background: "none",
              border: "none",
              padding: 0,
              cursor: "pointer",
              fontWeight: "600",
            }}
          >
            Create an account
          </button>
        </div>
      </form>
    </div>
  );
}

