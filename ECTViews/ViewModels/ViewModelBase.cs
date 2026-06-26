using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace ECTViews.ViewModels
{
    public abstract class ViewModelBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string name = null)
            => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));

        /// <summary>Signalisiert der UI, dass sich potenziell ALLE Properties
        /// geändert haben (leerer Name = "alles neu lesen"). Genutzt vom
        /// Einstellungs-Live-Sync, damit Änderungen aus einem anderen
        /// Dokumentfenster sofort sichtbar werden.</summary>
        public void RaiseAllPropertiesChanged()
            => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(string.Empty));

        protected bool SetProperty<T>(ref T field, T value,
            [CallerMemberName] string name = null)
        {
            if (Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(name);
            return true;
        }
    }
}
