package uk.co.mishurov.fluid

import android.os.Bundle
import android.app.ActionBar
import android.content.Intent
import android.view.MenuItem
import android.preference.PreferenceFragment
import android.preference.PreferenceActivity


class SettingsActivity : PreferenceActivity() {

    class SettingsFragment : PreferenceFragment() {
        override fun onCreate(savedInstanceState: Bundle?) {
            super.onCreate(savedInstanceState)
            addPreferencesFromResource(R.xml.preferences)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val actionBar = getActionBar()
        actionBar.setDisplayHomeAsUpEnabled(true)

        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, SettingsFragment())
                .commit()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        val mainIntent = Intent(getApplicationContext(), ParticlesActivity::class.java)
        startActivityForResult(mainIntent, 0)
        return true
    }

    companion object {
        private val TAG = "Fluid"
    }
}
