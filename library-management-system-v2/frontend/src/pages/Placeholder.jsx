export default function Placeholder({ title }) {
  return (
    <div className="card">
      <h1 className="title">{title}</h1>
      <p className="muted">This module is scaffolded and ready to connect to the C REST API.</p>
      <div style={{marginTop:20}}>
        <span className="badge badge-yellow">API integration pending</span>
      </div>
    </div>
  );
}
