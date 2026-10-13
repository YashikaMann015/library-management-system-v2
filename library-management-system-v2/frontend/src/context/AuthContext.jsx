import { createContext, useContext, useEffect, useMemo, useState } from "react";
import api from "../services/api";

const AuthContext = createContext(null);

export function AuthProvider({ children }) {
  const [user, setUser] = useState(() => {
    try { return JSON.parse(localStorage.getItem("library_user")) || null; } catch { return null; }
  });

  useEffect(() => {
    const token = localStorage.getItem("library_token");
    if (!token) return;
    api.get("/auth/me")
      .then(r => { if (r.data?.user) { setUser(r.data.user); localStorage.setItem("library_user", JSON.stringify(r.data.user)); }})
      .catch(() => {
        localStorage.removeItem("library_token");
        localStorage.removeItem("library_user");
        setUser(null);
      });
  }, []);

  const login = async (email, password) => {
    const { data } = await api.post("/auth/login", { email, password });
    if (!data.success) throw new Error(data.message || "Login failed");
    localStorage.setItem("library_token", data.token);
    localStorage.setItem("library_user", JSON.stringify(data.user));
    setUser(data.user);
    return data.user;
  };

  const logout = async () => {
    try { await api.post("/auth/logout"); } catch {}
    localStorage.removeItem("library_token");
    localStorage.removeItem("library_user");
    setUser(null);
  };

  const value = useMemo(() => ({ user, login, logout }), [user]);
  return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>;
}

export const useAuth = () => useContext(AuthContext);
