package uk.co.mishurov.fluid;

import android.os.Bundle;
import android.app.ActionBar;
import android.content.Intent;
import android.view.MenuItem;
import android.preference.PreferenceFragment;
import android.preference.PreferenceActivity;


public class SettingsActivity extends PreferenceActivity {
    private static final String TAG = "Fluid";

    public static class SettingsFragment extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.preferences);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);

        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new SettingsFragment())
                .commit();
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        Intent mainIntent = new Intent(getApplicationContext(), ParticlesActivity.class);
        startActivityForResult(mainIntent, 0);
        return true;
    }
}
